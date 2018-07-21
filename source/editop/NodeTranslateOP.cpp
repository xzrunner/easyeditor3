#include "ee3/NodeTranslateOP.h"
#include "ee3/CamRotateState.h"
#include "ee3/CamTranslateState.h"
#include "ee3/CamZoomState.h"
#include "ee3/NodeTranslate3State.h"

#include <ee0/WxStagePage.h>
#include <ee0/SubjectMgr.h>

namespace ee3
{

NodeTranslateOP::NodeTranslateOP(ee0::WxStagePage& stage, pt3::Camera& cam,
	                       const pt3::Viewport& vp)
	: m_sub_mgr(stage.GetSubjectMgr())
{
	m_cam_rotate_state    = std::make_shared<CamRotateState>(cam, m_sub_mgr);
	m_cam_translate_state = std::make_shared<CamTranslateState>(cam, m_sub_mgr);
	m_cam_zoom_state      = std::make_shared<CamZoomState>(cam, vp, m_sub_mgr);

	m_node_ranslate_state = std::make_shared<NodeTranslate3State>(cam, vp, m_sub_mgr, stage.GetSelection());

	m_op_state = m_cam_zoom_state;
}

bool NodeTranslateOP::OnMouseLeftDown(int x, int y)
{
	if (EditOP::OnMouseLeftDown(x, y)) {
		return true;
	}

	bool ret = false;
	if (m_node_ranslate_state->OnMousePress(x, y)) {
		m_op_state = m_node_ranslate_state;
		ret = true;
	} else {
		ChangeEditOpState(m_cam_rotate_state);
		ret = m_op_state->OnMousePress(x, y);
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

	return m_op_state->OnMouseRelease(x, y);
}

bool NodeTranslateOP::OnMouseRightDown(int x, int y)
{
	if (EditOP::OnMouseLeftDown(x, y)) {
		return true;
	}

	ChangeEditOpState(m_cam_translate_state);
	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

	return m_op_state->OnMousePress(x, y);
}

bool NodeTranslateOP::OnMouseRightUp(int x, int y)
{
	if (EditOP::OnMouseRightUp(x, y)) {
		return true;
	}

	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

	return 	m_op_state->OnMouseRelease(x, y);
}

bool NodeTranslateOP::OnMouseDrag(int x, int y)
{
	if (EditOP::OnMouseDrag(x, y)) {
		return true;
	}

	return m_op_state->OnMouseDrag(x, y);
}

bool NodeTranslateOP::OnMouseWheelRotation(int x, int y, int direction)
{
	if (EditOP::OnMouseWheelRotation(x, y, direction)) {
		return true;
	}

	return m_cam_zoom_state->OnMouseWheelRotation(x, y, direction);
}

bool NodeTranslateOP::OnActive()
{
	if (EditOP::OnActive()) {
		return true;
	}

	return m_node_ranslate_state->OnActive();
}

bool NodeTranslateOP::OnDraw() const
{
	if (EditOP::OnDraw()) {
		return true;
	}

	return m_node_ranslate_state->OnDraw();
}

void NodeTranslateOP::ChangeEditOpState(const ee0::EditOpStatePtr& state)
{
	if (m_op_state == state) {
		return;
	}

	if (m_op_state) {
		m_op_state->UnBind();
	}
	m_op_state = state;
	if (m_op_state) {
		m_op_state->Bind();
	}
}

}