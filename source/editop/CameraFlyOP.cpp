#include <ee3/CameraFlyOP.h>

#include <ee0/MessageID.h>
#include <ee0/SubjectMgr.h>

#include <painting3/PerspCam.h>

#include <Windows.h>
//#include <WinUser.h>
#include <wx/window.h>

namespace ee3
{

CameraFlyOP::CameraFlyOP(wxWindow* wnd, const std::shared_ptr<pt0::Camera>& camera,
	                     const ee0::SubjectMgrPtr& sub_mgr)
	: CameraMoveOP(camera, sub_mgr)
	, m_wnd(wnd)
	, m_sub_mgr(sub_mgr)
{
	m_last_pos.MakeInvalid();
}

bool CameraFlyOP::OnMouseMove(int x, int y)
{
	ShowCursor(0);

	if (CameraMoveOP::OnMouseMove(x, y)) {
		return true;
	}

	if (!m_last_pos.IsValid()) {
		m_last_pos.Set(x, y);
		return false;
	}

	auto cam_type = m_camera->TypeID();
	if (cam_type == pt0::GetCamTypeID<pt3::PerspCam>())
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

	// force set cursor to center
	auto sz = m_wnd->GetSize();
	x = sz.x * 0.5f;
	y = sz.y * 0.5f;
	SetCursorPos(x + m_wnd->GetScreenPosition().x, y + m_wnd->GetScreenPosition().y);

	m_last_pos.Set(x, y);

	return false;
}

}