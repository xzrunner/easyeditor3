#include "ee3/NodeTranslateOP.h"
#include "ee3/NodeTranslate3State.h"

#include <ee0/WxStagePage.h>
#include <ee0/SubjectMgr.h>

#include <painting3/PerspCam.h>

namespace ee3
{

NodeTranslateOP::NodeTranslateOP(ee0::WxStagePage& stage, pt3::PerspCam& cam,
	                             const pt3::Viewport& vp)
	: m_sub_mgr(stage.GetSubjectMgr())
{
	m_translate_state = std::make_shared<NodeTranslate3State>(cam, vp, m_sub_mgr, stage.GetSelection());
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

	return m_translate_state->OnActive();
}

bool NodeTranslateOP::OnDraw() const
{
	if (EditOP::OnDraw()) {
		return true;
	}

	return m_translate_state->OnDraw();
}

}