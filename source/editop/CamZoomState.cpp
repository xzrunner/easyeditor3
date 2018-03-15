#include "ee3/CamZoomState.h"

#include <ee0/SubjectMgr.h>

#include <painting3/Camera.h>
#include <painting3/Viewport.h>

namespace ee3
{

CamZoomState::CamZoomState(pt3::Camera& cam, const pt3::Viewport& vp,
	                       const ee0::SubjectMgrPtr& sub_mgr)
	: m_cam(cam)
	, m_vp(vp)
	, m_sub_mgr(sub_mgr)
{
}

bool CamZoomState::OnMouseWheelRotation(int x, int y, int direction)
{
	sm::vec2 pos(static_cast<float>(x), static_cast<float>(y));
	sm::vec3 dir = m_vp.TransPos3ScreenToDir(pos, m_cam);
	static const float OFFSET = 0.05f;
	if (direction > 0) {
		m_cam.Move(dir, m_cam.GetDistance() * OFFSET);
	} else {
		m_cam.Move(dir, - m_cam.GetDistance() * OFFSET);
	}

	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

	return false;
}

}