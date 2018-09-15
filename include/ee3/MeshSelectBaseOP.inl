#pragma once

#include "ee3/MeshSelectBaseOP.h"
#include "ee3/MeshEditStyle.h"

#include <ee0/SubjectMgr.h>
#include <ee0/MessageID.h>
#include <ee2/DrawSelectRectState.h>

#include <painting2/OrthoCamera.h>
#include <painting2/PrimitiveDraw.h>
#include <painting3/Viewport.h>
#include <SM_Calc.h>
#include <model/Model.h>
#include <model/MapLoader.h>

#include <wx/utils.h>

namespace ee3
{
namespace mesh
{

template <typename T>
MeshSelectBaseOP<T>::MeshSelectBaseOP(const std::shared_ptr<pt0::Camera>& camera, const pt3::Viewport& vp,
	                                  const ee0::SubjectMgrPtr& sub_mgr,
	                                  const MeshPointQuery::Selected& selected)
	: ee0::EditOP(camera)
	, m_vp(vp)
	, m_sub_mgr(sub_mgr)
	, m_base_selected(selected)
	, m_cam2d(std::make_shared<pt2::OrthoCamera>())
{
	m_cam2d->OnSize(m_vp.Width(), m_vp.Height());

	m_draw_state = std::make_unique<ee2::DrawSelectRectState>(m_cam2d, m_sub_mgr);
}

template <typename T>
bool MeshSelectBaseOP<T>::OnMouseLeftDown(int x, int y)
{
	if (ee0::EditOP::OnMouseLeftDown(x, y)) {
		return true;
	}

	if (m_draw_state_enable) {
		m_draw_state->OnMousePress(x, y);
	}

	// todo
	if (!m_base_selected.poly) {
		return false;
	}

	m_first_pos.Set(x, y);

	// select
	sm::ivec2 end_pos(x, y);
	if (m_first_pos == end_pos) {
		SelectByPos(end_pos);
	} else {
		SelectByRect(sm::irect(m_first_pos, end_pos));
	}

	return false;
}

template <typename T>
bool MeshSelectBaseOP<T>::OnMouseLeftUp(int x, int y)
{
	if (ee0::EditOP::OnMouseLeftUp(x, y)) {
		return true;
	}

	//// select
	//sm::ivec2 end_pos(x, y);
	//if (m_first_pos == end_pos) {
	//	SelectByPos(end_pos);
	//} else {
	//	SelectByRect(sm::irect(m_first_pos, end_pos));
	//}

	// draw state
	if (m_draw_state_enable) {
		m_draw_state->OnMouseRelease(x, y);
	}
	m_draw_state_enable = true;
	m_draw_state->Clear();

	return false;
}

template <typename T>
bool MeshSelectBaseOP<T>::OnMouseMove(int x, int y)
{
	if (ee0::EditOP::OnMouseMove(x, y)) {
		return true;
	}

	// todo
	if (!m_base_selected.poly) {
		return false;
	}

	auto selecting = QueryByPos(x, y);
	if (m_selecting != selecting) {
		m_selecting = QueryByPos(x, y);
		m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
	}

	return false;
}

template <typename T>
bool MeshSelectBaseOP<T>::OnMouseDrag(int x, int y)
{
	if (ee0::EditOP::OnMouseDrag(x, y)) {
		return true;
	}

	if (m_draw_state_enable) {
		m_draw_state->OnMouseDrag(x, y);
	}

	return false;
}

template <typename T>
bool MeshSelectBaseOP<T>::OnDraw() const
{
	if (ee0::EditOP::OnDraw()) {
		return true;
	}

	auto brush = m_base_selected.GetBrush();
	if (!brush || m_base_selected.brush_idx < 0) 
	{
		if (m_draw_state_enable) {
			m_draw_state->OnDraw();
		}
		return false;
	}

	auto cam_mat = m_camera->GetViewMat() * m_camera->GetProjectionMat();
	DrawImpl(*brush, cam_mat);

	if (m_draw_state_enable) {
		m_draw_state->OnDraw();
	}

	return false;
}

template <typename T>
void MeshSelectBaseOP<T>::SelectByPos(const sm::ivec2& pos)
{
	auto selected = QueryByPos(pos.x, pos.y);
	if (selected) {
		m_draw_state_enable = false;
	}

	if (!selected) 
	{
		m_selected.Clear();
	} 
	else 
	{
		if (m_selected.IsEmpty()) 
		{
			m_selected.Add(selected);
		} 
		else 
		{
			if (wxGetKeyState(WXK_CONTROL))
			{
				if (m_selected.IsExist(selected)) {
					m_selected.Remove(selected);
				} else {
					m_selected.Add(selected);
				}
			}
			else
			{
				if (!m_selected.IsExist(selected)) {
					m_selected.Clear();
					m_selected.Add(selected);
				}
			}
		}
	}
}

template <typename T>
void MeshSelectBaseOP<T>::SelectByRect(const sm::irect& rect)
{
	std::vector<T> selection;
	QueryByRect(rect, selection);
	if (wxGetKeyState(WXK_CONTROL))
	{
		for (auto& v : selection) 
		{
			if (m_selected.IsExist(v)) {
				m_selected.Remove(v);
			} else {
				m_selected.Add(v);
			}
		}
	}
	else
	{
		m_selected.Clear();
		for (auto& v : selection) {
			m_selected.Add(v);
		}
	}
}

}
}