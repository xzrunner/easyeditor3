#include "ee3/NodeTranslateOP.h"
#include "ee3/NodeTranslate3State.h"

#include <ee0/WxStagePage.h>
#include <ee0/SubjectMgr.h>

#include <painting3/PerspCam.h>

namespace ee3
{

NodeTranslateOP::NodeTranslateOP(const std::shared_ptr<pt0::Camera>& camera,
	                             ee0::WxStagePage& stage, const pt3::Viewport& vp)
	: ee0::EditOP(camera)
	, m_sub_mgr(stage.GetSubjectMgr())
{
	assert(camera->TypeID() == pt0::GetCamTypeID<pt3::PerspCam>());
	auto p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);
	m_translate_state = std::make_shared<NodeTranslate3State>(
		p_cam, vp, m_sub_mgr, stage.GetSelection());
}

bool NodeTranslateOP::OnMouseLeftDown(int x, int y)
{
	bool ret = false;
	if (m_translate_state->OnMousePress(x, y)) {
		ret = true;
	} else {
		ret = EditOP::OnMouseLeftDown(x, y);
	}

	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

	return ret;
}

bool NodeTranslateOP::OnMouseLeftUp(int x, int y)
{
	if (EditOP::OnMouseLeftUp(x, y)) {
		return true;
	}

	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

	return m_translate_state->OnMouseRelease(x, y);
}

bool NodeTranslateOP::OnMouseDrag(int x, int y)
{
	if (EditOP::OnMouseDrag(x, y)) {
		return true;
	}

	return m_translate_state->OnMouseDrag(x, y);
}

bool NodeTranslateOP::OnActive()
{
	if (EditOP::OnActive()) {
		return true;
	}

	return m_translate_state->OnActive(true);
}

bool NodeTranslateOP::OnDraw() const
{
	if (EditOP::OnDraw()) {
		return true;
	}

	return m_translate_state->OnDraw();
}

}