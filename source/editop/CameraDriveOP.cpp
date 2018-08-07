#include "ee3/CameraDriveOP.h"
#include "ee3/CameraMgr.h"

#include <ee0/MessageID.h>
#include <ee0/SubjectMgr.h>

#include <painting3/OrthoCam.h>
#include <painting3/PerspCam.h>
#include <painting3/Viewport.h>

namespace ee3
{

CameraDriveOP::CameraDriveOP(const std::shared_ptr<pt0::Camera>& camera,
	                         const pt3::Viewport& vp,
	                         const ee0::SubjectMgrPtr& sub_mgr)
	: CameraMoveOP(camera, sub_mgr)
	, m_vp(vp)
	, m_sub_mgr(sub_mgr)
{
	m_last_pos.MakeInvalid();
}

bool CameraDriveOP::OnMouseRightDown(int x, int y)
{
	if (CameraMoveOP::OnMouseRightDown(x, y)) {
		return true;
	}

	m_last_pos.Set(x, y);
	return false;
}

bool CameraDriveOP::OnMouseRightUp(int x, int y)
{
	if (CameraMoveOP::OnMouseRightUp(x, y)) {
		return true;
	}

	m_last_pos.MakeInvalid();
	return false;
}

bool CameraDriveOP::OnMouseDrag(int x, int y)
{
	if (CameraMoveOP::OnMouseDrag(x, y)) {
		return true;
	}

	if (!m_last_pos.IsValid()) {
		return false;
	}

	auto cam_type = m_camera->TypeID();
	if (cam_type == pt0::GetCamTypeID<pt3::OrthoCam>())
	{
		auto& o_cam = std::dynamic_pointer_cast<pt3::OrthoCam>(m_camera);

		float dx = static_cast<float>(m_last_pos.x - x);
		float dy = static_cast<float>(y - m_last_pos.y);
		o_cam->Translate(sm::vec2(dx, dy));

		m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
	}
	else if (cam_type == pt0::GetCamTypeID<pt3::PerspCam>())
	{
		auto& p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);

		int dx = x - m_last_pos.x;
		int dy = y - m_last_pos.y;

		static const float SCALE = 0.5f;
		p_cam->Yaw(-dx * SCALE);
		p_cam->Pitch(-dy * SCALE);
		p_cam->SetUpDir(sm::vec3(0, 1, 0));

		m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
	}

	m_last_pos.Set(x, y);

	return false;
}

bool CameraDriveOP::OnMouseWheelRotation(int x, int y, int direction)
{
	if (CameraMoveOP::OnMouseWheelRotation(x, y, direction)) {
		return true;
	}

	auto cam_type = m_camera->TypeID();
	if (cam_type == pt0::GetCamTypeID<pt3::OrthoCam>())
	{
		auto& o_cam = std::dynamic_pointer_cast<pt3::OrthoCam>(m_camera);

		float w = m_vp.Width(),
			  h = m_vp.Height();
		float scale = direction > 0 ? 1 / 1.1f : 1.1f;
		const float cx = static_cast<float>(x),
				    cy = static_cast<float>(h - y);
		o_cam->Scale(scale, cx, cy, w, h);

		m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
	}
	else if (cam_type == pt0::GetCamTypeID<pt3::PerspCam>())
	{
		auto& p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);

		sm::vec2 pos(static_cast<float>(x), static_cast<float>(y));
		sm::vec3 dir = m_vp.TransPos3ScreenToDir(pos, *p_cam);
		static const float OFFSET = 0.05f;
		if (direction > 0) {
			p_cam->Move(dir, p_cam->GetDistance() * OFFSET);
		} else {
			p_cam->Move(dir, -p_cam->GetDistance() * OFFSET);
		}

		m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
	}

	return false;
}

}