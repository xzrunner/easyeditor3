#include "ee3/CamRotateState.h"

#include <ee0/SubjectMgr.h>
#include <ee0/MessageID.h>

#include <SM_Calc.h>
#include <painting3/PerspCam.h>

namespace ee3
{

CamRotateState::CamRotateState(const std::shared_ptr<pt0::Camera>& camera, 
	                           const ee0::SubjectMgrPtr& sub_mgr)
	: ee0::EditOpState(camera)
	, m_sub_mgr(sub_mgr)
{
	m_last_pos.MakeInvalid();
}

bool CamRotateState::OnMousePress(int x, int y)
{
	m_last_pos.Set(x, y);
	return false;
}

bool CamRotateState::OnMouseRelease(int x, int y)
{
	return false;
}

bool CamRotateState::OnMouseDrag(int x, int y)
{
	if (m_camera->TypeID() == pt0::GetCamTypeID<pt3::PerspCam>())
	{
		auto p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);

		int dx = x - m_last_pos.x;
		int dy = y - m_last_pos.y;

		p_cam->Yaw(-dx * 0.2f);
		p_cam->Pitch(-dy * 0.2f);
		p_cam->AimAtTarget();
		p_cam->SetUpDir(sm::vec3(0, 1, 0));

		m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
	}

	m_last_pos.Set(x, y);

	return false;
}

}