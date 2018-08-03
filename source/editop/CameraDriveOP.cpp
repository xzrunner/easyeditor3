#include "ee3/CameraDriveOP.h"
#include "ee3/CameraMgr.h"

#include <ee0/MessageID.h>
#include <ee0/SubjectMgr.h>

#include <painting3/OrthoCam.h>
#include <painting3/PerspCam.h>
#include <painting3/Viewport.h>

#include <wx/defs.h>
#include <wx/utils.h>

namespace ee3
{

CameraDriveOP::CameraDriveOP(CameraMgr& cam_mgr, const pt3::Viewport& vp,
	                         const ee0::SubjectMgrPtr& sub_mgr)
	: m_cam_mgr(cam_mgr)
	, m_vp(vp)
	, m_sub_mgr(sub_mgr)
{
	m_last_pos.MakeInvalid();
}

bool CameraDriveOP::OnKeyDown(int key_code)
{
	if (ee0::EditOP::OnKeyDown(key_code)) { return true; }

	switch (key_code)
	{
	case WXK_ESCAPE:
		m_cam_mgr.GetCamera()->Reset();
		m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
		break;
	case 'w': case 'W':
		m_move_dir = MOVE_UP;
		break;
	case 's': case 'S':
		m_move_dir = MOVE_DOWN;
		break;
	case 'a': case 'A':
		m_move_dir = MOVE_LEFT;
		break;
	case 'd': case 'D':
		m_move_dir = MOVE_RIGHT;
		break;
	}

	return false;
}

bool CameraDriveOP::OnKeyUp(int key_code)
{
	if (ee0::EditOP::OnKeyUp(key_code)) { return true; }

	switch (key_code)
	{
		case 'W':
		case 'S':
		case 'A':
		case 'D':
			m_move_dir = MOVE_NONE;
	}

	return false;
}

bool CameraDriveOP::OnMouseRightDown(int x, int y)
{
	if (ee0::EditOP::OnMouseRightDown(x, y)) {
		return true;
	}

	m_last_pos.Set(x, y);
	return false;
}

bool CameraDriveOP::OnMouseRightUp(int x, int y)
{
	if (ee0::EditOP::OnMouseRightUp(x, y)) {
		return true;
	}

	m_last_pos.MakeInvalid();
	return false;
}

bool CameraDriveOP::OnMouseDrag(int x, int y)
{
	if (ee0::EditOP::OnMouseDrag(x, y)) {
		return true;
	}

	if (!m_last_pos.IsValid()) {
		return false;
	}

	auto& cam = m_cam_mgr.GetCamera();
	auto cam_type = cam->TypeID();
	if (cam_type == pt0::GetCamTypeID<pt3::OrthoCam>())
	{
		auto& o_cam = std::dynamic_pointer_cast<pt3::OrthoCam>(cam);

		auto dx = m_last_pos.x - x;
		auto dy = y - m_last_pos.y;
		o_cam->Translate(sm::vec2(dx, dy));

		m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
	}
	else if (cam_type == pt0::GetCamTypeID<pt3::PerspCam>())
	{
		auto& p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(cam);

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
	if (ee0::EditOP::OnMouseWheelRotation(x, y, direction)) {
		return true;
	}

	auto& cam = m_cam_mgr.GetCamera();
	auto cam_type = cam->TypeID();
	if (cam_type == pt0::GetCamTypeID<pt3::OrthoCam>())
	{
		auto& o_cam = std::dynamic_pointer_cast<pt3::OrthoCam>(cam);

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
		auto& p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(cam);

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

bool CameraDriveOP::Update(float dt)
{
	if (m_move_dir == MOVE_NONE) {
		return false;
	}

	static const float SPEED = 5.0f;
	float offset = SPEED * dt;
	if (wxGetKeyState(WXK_SHIFT)) {
		offset *= 0.1f;
	}

	auto& cam = m_cam_mgr.GetCamera();
	auto cam_type = cam->TypeID();
	if (cam_type == pt0::GetCamTypeID<pt3::OrthoCam>())
	{
	}
	else if (cam_type == pt0::GetCamTypeID<pt3::PerspCam>())
	{
		auto& p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(cam);

		switch (m_move_dir)
		{
		case MOVE_LEFT:
			p_cam->Translate(offset, 0);
			break;
		case MOVE_RIGHT:
			p_cam->Translate(-offset, 0);
			break;
		case MOVE_UP:
			p_cam->MoveToward(offset);
			break;
		case MOVE_DOWN:
			p_cam->MoveToward(-offset);
			break;
		}

		m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
	}

	return true;
}

}