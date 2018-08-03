#include "ee3/CamTranslateState.h"

#include <ee0/SubjectMgr.h>
#include <ee0/MessageID.h>

#include <painting3/PerspCam.h>

namespace ee3
{

CamTranslateState::CamTranslateState(const std::shared_ptr<pt0::Camera>& camera, 
	                                 const ee0::SubjectMgrPtr& sub_mgr)
	: ee0::EditOpState(camera)
	, m_sub_mgr(sub_mgr)
{
	m_last_pos.MakeInvalid();
}

bool CamTranslateState::OnMousePress(int x, int y)
{
	m_last_pos.Set(x, y);
	return false;
}

bool CamTranslateState::OnMouseRelease(int x, int y)
{
	return false;
}

bool CamTranslateState::OnMouseDrag(int x, int y)
{
	if (m_camera->TypeID() == pt0::GetCamTypeID<pt3::PerspCam>())
	{
		auto p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);

		int dx = x - m_last_pos.x;
		int dy = y - m_last_pos.y;

		float dist = p_cam->GetDistance();
		static const float SCALE = 0.002f;
		p_cam->Translate(dx * dist * SCALE, dy * dist * SCALE);

		m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
	}

	m_last_pos.Set(x, y);

	return false;
}

}