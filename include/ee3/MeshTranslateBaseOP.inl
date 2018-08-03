#pragma once

#include "ee3/MeshTranslateBaseOP.h"
#include "ee3/MeshEditStyle.h"

#include <ee0/MessageID.h>
#include <ee0/SubjectMgr.h>

#include <painting2/OrthoCamera.h>
#include <painting3/PerspCam.h>
#include <painting3/Viewport.h>
#include <model/MapLoader.h>
#include <SM_Calc.h>
#include <SM_Ray.h>
#include <SM_RayIntersect.h>

#include <wx/defs.h>

namespace ee3
{
namespace mesh
{

template <typename T>
MeshTranslateBaseOP<T>::MeshTranslateBaseOP(const std::shared_ptr<pt0::Camera>& camera,
	                                        const pt3::Viewport& vp,
	                                        const ee0::SubjectMgrPtr& sub_mgr,
	                                        const MeshPointQuery::Selected& selected,
	                                        const ee0::SelectionSet<T>& selection)
	: ee0::EditOP(camera)
	, m_vp(vp)
	, m_sub_mgr(sub_mgr)
	, m_selected(selected)
	, m_selection(selection)
	, m_cam2d(std::make_shared<pt2::OrthoCamera>())
{
	m_cam2d->OnSize(m_vp.Width(), m_vp.Height());

	m_last_pos.MakeInvalid();
}

template <typename T>
bool MeshTranslateBaseOP<T>::OnKeyDown(int key_code)
{
	if (ee0::EditOP::OnKeyDown(key_code)) {
		return true;
	}

	const float offset = 0.001f;
	switch (key_code)
	{
	case WXK_LEFT:
		TranslateSelected(sm::vec3(-offset, 0, 0));
		break;
	case WXK_RIGHT:
		TranslateSelected(sm::vec3(offset, 0, 0));
		break;
	case WXK_DOWN:
		TranslateSelected(sm::vec3(0, 0, -offset));
		break;
	case WXK_UP:
		TranslateSelected(sm::vec3(0, 0, offset));
		break;
	case WXK_PAGEDOWN:
		TranslateSelected(sm::vec3(0, -offset, 0));
		break;
	case WXK_PAGEUP:
		TranslateSelected(sm::vec3(0, offset, 0));
		break;
	}

	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

	return false;
}

template <typename T>
bool MeshTranslateBaseOP<T>::OnMouseLeftDown(int x, int y)
{
	if (ee0::EditOP::OnMouseLeftDown(x, y)) {
		return true;
	}

	if (m_selection.IsEmpty()) {
		return false;
	}

	m_last_pos.MakeInvalid();

	auto pos = m_cam2d->TransPosScreenToProject(x, y,
		static_cast<int>(m_vp.Width()), static_cast<int>(m_vp.Height()));
	auto cam_mat = m_camera->GetModelViewMat() * m_camera->GetProjectionMat();
	m_selection.Traverse([&](const T& data)->bool {
		return !QueryByPos(pos, data, cam_mat);
	});

	return false;
}

template <typename T>
bool MeshTranslateBaseOP<T>::OnMouseLeftUp(int x, int y)
{
	if (ee0::EditOP::OnMouseLeftUp(x, y)) {
		return true;
	}

	m_last_pos.MakeInvalid();

	return false;
}

template <typename T>
bool MeshTranslateBaseOP<T>::OnMouseDrag(int x, int y)
{
	if (ee0::EditOP::OnMouseDrag(x, y)) {
		return true;
	}

	if (!m_last_pos.IsValid()) {
		return false;
	}

	if (m_camera->TypeID() == pt0::GetCamTypeID<pt3::PerspCam>())
	{
		auto p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);

		sm::vec3 ray_dir = m_vp.TransPos3ScreenToDir(
			sm::vec2(static_cast<float>(x), static_cast<float>(y)), *p_cam);
		sm::Ray ray(p_cam->GetPos(), ray_dir);

		sm::Plane plane(sm::vec3(0, 1, 0), -m_last_pos.y);
		sm::vec3 cross;
		if (!sm::ray_plane_intersect(ray, plane, &cross)) {
			return false;
		}

		TranslateSelected(cross - m_last_pos);
		m_last_pos = cross;

		m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
	}

	return false;
}

}
}