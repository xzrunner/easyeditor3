#include "ee3/WorldTravelOP.h"
#include "ee3/CamRotateState.h"
#include "ee3/CamTranslateState.h"
#include "ee3/CamZoomState.h"

#include <ee0/MessageID.h>
#include <ee0/SubjectMgr.h>

#include <painting3/PerspCam.h>

#include <wx/defs.h>

namespace ee3
{

static const float MOUSE_SENSITIVITY = 0.3f;

WorldTravelOP::WorldTravelOP(const std::shared_ptr<pt0::Camera>& camera,
	                         const pt3::Viewport& vp,
	                         const ee0::SubjectMgrPtr& sub_mgr,
                             bool only_zoom)
	: ee0::EditOP(camera)
	, m_sub_mgr(sub_mgr)
    , m_only_zoom(only_zoom)
{
	assert(camera->TypeID() == pt0::GetCamTypeID<pt3::PerspCam>());
	auto p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(camera);
	m_rotate_state    = std::make_shared<CamRotateState>(p_cam, sub_mgr);
	m_translate_state = std::make_shared<CamTranslateState>(p_cam, sub_mgr);
	m_zoom_state      = std::make_shared<CamZoomState>(p_cam, vp, sub_mgr);
	m_op_state = m_zoom_state;
}

bool WorldTravelOP::OnKeyDown(int key_code)
{
	if (ee0::EditOP::OnKeyDown(key_code)) { return true; }

	static const float OFFSET = 0.1f;

	switch (key_code)
	{
	case WXK_ESCAPE:
		m_camera->Reset();
		m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
		break;
	}

	if (m_camera->TypeID() == pt0::GetCamTypeID<pt3::PerspCam>())
	{
		auto p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);
		switch (key_code)
		{
		case WXK_ESCAPE:
			m_camera->Reset();
			m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
			break;
		case 'w': case 'W':
			p_cam->Translate(0, OFFSET);
			m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
			break;
		case 's': case 'S':
			p_cam->Translate(0, -OFFSET);
			m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
			break;
 		case 'a': case 'A':
			p_cam->Translate(OFFSET, 0);
			m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
 			break;
 		case 'd': case 'D':
			p_cam->Translate(-OFFSET, 0);
			m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
 			break;
		}
	}

	return false;
}

bool WorldTravelOP::OnMouseLeftDown(int x, int y)
{
	if (ee0::EditOP::OnMouseLeftDown(x, y)) {
		return true;
	}

    if (!m_only_zoom) {
        ChangeEditOpState(m_rotate_state);
    }
	return m_op_state->OnMousePress(x, y);
}

bool WorldTravelOP::OnMouseLeftUp(int x, int y)
{
	if (ee0::EditOP::OnMouseLeftDown(x, y)) {
		return true;
	}

	ChangeEditOpState(m_zoom_state);

	return false;
}

bool WorldTravelOP::OnMouseRightDown(int x, int y)
{
	if (ee0::EditOP::OnMouseLeftDown(x, y)) {
		return true;
	}

    if (!m_only_zoom) {
        ChangeEditOpState(m_translate_state);
    }
	return m_op_state->OnMousePress(x, y);
}

bool WorldTravelOP::OnMouseRightUp(int x, int y)
{
	if (ee0::EditOP::OnMouseRightUp(x, y)) {
		return true;
	}

	ChangeEditOpState(m_zoom_state);

	return false;
}

bool WorldTravelOP::OnMouseDrag(int x, int y)
{
	if (ee0::EditOP::OnMouseDrag(x, y)) {
		return true;
	}

	return m_op_state->OnMouseDrag(x, y);
}

bool WorldTravelOP::OnMouseMove(int x, int y)
{
	if (ee0::EditOP::OnMouseMove(x, y)) {
		return true;
	}

	return m_op_state->OnMouseMove(x, y);
}

bool WorldTravelOP::OnMouseWheelRotation(int x, int y, int direction)
{
	if (ee0::EditOP::OnMouseWheelRotation(x, y, direction)) {
		return true;
	}

	return m_op_state->OnMouseWheelRotation(x, y, direction);
}

}