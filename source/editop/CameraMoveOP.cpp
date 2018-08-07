#include "ee3/CameraMoveOP.h"

#include <ee0/MessageID.h>
#include <ee0/SubjectMgr.h>

#include <painting3/OrthoCam.h>
#include <painting3/PerspCam.h>

#include <wx/utils.h>

namespace ee3
{

CameraMoveOP::CameraMoveOP(const std::shared_ptr<pt0::Camera>& camera,
	                       const ee0::SubjectMgrPtr& sub_mgr)
	: ee0::EditOP(camera)
	, m_sub_mgr(sub_mgr)
{
}

bool CameraMoveOP::OnKeyDown(int key_code)
{
	if (ee0::EditOP::OnKeyDown(key_code)) {
		return true;
	}

	switch (key_code)
	{
	case WXK_ESCAPE:
		m_camera->Reset();
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

bool CameraMoveOP::OnKeyUp(int key_code)
{
	if (ee0::EditOP::OnKeyUp(key_code)) {
		return true;
	}

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

bool CameraMoveOP::Update(float dt)
{
	if (m_move_dir == MOVE_NONE) {
		return false;
	}

	static const float SPEED = 5.0f;
	float offset = SPEED * dt;
	if (wxGetKeyState(WXK_SHIFT)) {
		offset *= 0.1f;
	}

	auto cam_type = m_camera->TypeID();
	if (cam_type == pt0::GetCamTypeID<pt3::OrthoCam>())
	{
	}
	else if (cam_type == pt0::GetCamTypeID<pt3::PerspCam>())
	{
		auto& p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);

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