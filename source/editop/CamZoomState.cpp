#include "ee3/CamZoomState.h"

#include <ee0/SubjectMgr.h>
#include <ee0/MessageID.h>

#include <painting3/PerspCam.h>
#include <painting3/Viewport.h>

namespace ee3
{

CamZoomState::CamZoomState(const std::shared_ptr<pt0::Camera>& camera, 
	                       const pt3::Viewport& vp,
	                       const ee0::SubjectMgrPtr& sub_mgr)
	: ee0::EditOpState(camera)
	, m_vp(vp)
	, m_sub_mgr(sub_mgr)
{
}

bool CamZoomState::OnMouseWheelRotation(int x, int y, int direction)
{
	if (m_camera->TypeID() == pt0::GetCamTypeID<pt3::PerspCam>())
	{
		auto p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);

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