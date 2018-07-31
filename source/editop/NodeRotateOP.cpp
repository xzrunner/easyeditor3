#include "ee3/NodeRotateOP.h"
#include "ee3/NodeRotate3State.h"

#include <ee0/WxStagePage.h>
#include <ee0/SubjectMgr.h>

namespace ee3
{

NodeRotateOP::NodeRotateOP(ee0::WxStagePage& stage, pt3::Camera& cam,
	                       const pt3::Viewport& vp)
	: m_sub_mgr(stage.GetSubjectMgr())
{
	m_rotate_state = std::make_shared<NodeRotate3State>(cam, vp, m_sub_mgr, stage.GetSelection());
}

bool NodeRotateOP::OnMouseLeftDown(int x, int y)
{
	bool ret = false;
	if (m_rotate_state->OnMousePress(x, y)) {
		ret = true;
	} else {
		ret = EditOP::OnMouseLeftDown(x, y);
	}

	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

	return ret;
}

bool NodeRotateOP::OnMouseLeftUp(int x, int y)
{
	if (EditOP::OnMouseLeftUp(x, y)) {
		return true;
	}

	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

	return m_rotate_state->OnMouseRelease(x, y);
}

bool NodeRotateOP::OnMouseDrag(int x, int y)
{
	if (EditOP::OnMouseDrag(x, y)) {
		return true;
	}

	return m_rotate_state->OnMouseDrag(x, y);
}

bool NodeRotateOP::OnActive()
{
	if (EditOP::OnActive()) {
		return true;
	}

	return m_rotate_state->OnActive();
}

bool NodeRotateOP::OnDraw() const
{
	if (EditOP::OnDraw()) {
		return true;
	}

	return m_rotate_state->OnDraw();
}

}