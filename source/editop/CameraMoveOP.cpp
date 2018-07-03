#include "ee3/CameraMoveOP.h"

#include <ee0/MessageID.h>
#include <ee0/SubjectMgr.h>

#include <painting3/Camera.h>
#include <painting3/Viewport.h>

#include <wx/defs.h>

namespace ee3
{

CameraMoveOP::CameraMoveOP(pt3::Camera& cam, const pt3::Viewport& vp,
	                       const ee0::SubjectMgrPtr& sub_mgr)
	: ee0::EditOP()
	, m_cam(cam)
	, m_vp(vp)
	, m_sub_mgr(sub_mgr)
{
	m_last_pos.MakeInvalid();
}

bool CameraMoveOP::OnKeyDown(int key_code)
{
	if (ee0::EditOP::OnKeyDown(key_code)) { return true; }

	static const float OFFSET = 0.1f;

	switch (key_code)
	{
	case WXK_ESCAPE:
		m_cam.Reset();
		m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
		break;
	case 'w': case 'W':
		m_cam.MoveToward(OFFSET);
		m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
		break;
	case 's': case 'S':
		m_cam.MoveToward(-OFFSET);
		m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
		break;
	case 'a': case 'A':
		m_cam.Translate(OFFSET, 0);
		m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
		break;
	case 'd': case 'D':
		m_cam.Translate(-OFFSET, 0);
		m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
		break;
	}

	return false;
}

bool CameraMoveOP::OnMouseRightDown(int x, int y)
{
	if (ee0::EditOP::OnMouseRightDown(x, y)) {
		return true;
	}

	m_last_pos.Set(x, y);
	return false;
}

bool CameraMoveOP::OnMouseRightUp(int x, int y)
{
	if (ee0::EditOP::OnMouseRightUp(x, y)) {
		return true;
	}

	m_last_pos.MakeInvalid();
	return false;
}

bool CameraMoveOP::OnMouseDrag(int x, int y)
{
	if (ee0::EditOP::OnMouseDrag(x, y)) {
		return true;
	}

	if (!m_last_pos.IsValid()) {
		return false;
	}

	int dx = x - m_last_pos.x;
	int dy = y - m_last_pos.y;

	static const float SCALE = 0.5f;
	m_cam.Yaw(-dx * SCALE);
	m_cam.Pitch(-dy * SCALE);
	m_cam.SetUpDir(sm::vec3(0, 1, 0));

	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

	m_last_pos.Set(x, y);

	return false;
}

bool CameraMoveOP::OnMouseWheelRotation(int x, int y, int direction)
{
	if (ee0::EditOP::OnMouseWheelRotation(x, y, direction)) {
		return true;
	}

	sm::vec2 pos(static_cast<float>(x), static_cast<float>(y));
	sm::vec3 dir = m_vp.TransPos3ScreenToDir(pos, m_cam);
	static const float OFFSET = 0.05f;
	if (direction > 0) {
		m_cam.Move(dir, m_cam.GetDistance() * OFFSET);
	}
	else {
		m_cam.Move(dir, -m_cam.GetDistance() * OFFSET);
	}

	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

	return false;
}

}