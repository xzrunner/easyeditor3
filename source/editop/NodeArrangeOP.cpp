#include "ee3/NodeArrangeOP.h"
#include "ee3/WxStageCanvas.h"
#include "ee3/CamTranslateState.h"
#include "ee3/CamRotateState.h"
#include "ee3/CamZoomState.h"
#include "ee3/NodeTranslateState.h"
#include "ee3/NodeRotateState.h"

#include <ee0/WxStagePage.h>
#include <ee0/SubjectMgr.h>

#include <painting3/PerspCam.h>
#ifndef GAME_OBJ_ECS
#include <node0/SceneNode.h>
#include <node3/CompTransform.h>
#endif // GAME_OBJ_ECS

namespace ee3
{

NodeArrangeOP::NodeArrangeOP(const std::shared_ptr<pt0::Camera>& camera,
	                         ee0::WxStagePage& stage,
	                         const pt3::Viewport& vp)
	: NodeSelectOP(camera, stage, vp)
	, m_sub_mgr(stage.GetSubjectMgr())
	, m_selection(stage.GetSelection())
	, m_canvas(std::dynamic_pointer_cast<WxStageCanvas>(stage.GetImpl().GetCanvas()))
{
	assert(camera->TypeID() == pt0::GetCamTypeID<pt3::PerspCam>());
	auto p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(camera);

	m_cam_rotate_state    = std::make_shared<CamRotateState>(p_cam, m_sub_mgr);
	m_cam_translate_state = std::make_shared<CamTranslateState>(p_cam, m_sub_mgr);
	m_cam_zoom_state      = std::make_shared<CamZoomState>(p_cam, vp, m_sub_mgr);

	m_node_rotate_state    = std::make_shared<NodeRotateState>(p_cam, vp, m_sub_mgr, m_selection);
	m_node_translate_state = std::make_shared<NodeTranslateState>(p_cam, vp, m_sub_mgr, m_selection);

	m_op_state = m_cam_rotate_state;

	m_last_left_press.MakeInvalid();
	m_last_right_press.MakeInvalid();
}

bool NodeArrangeOP::OnKeyDown(int key_code)
{
	if (NodeSelectOP::OnKeyDown(key_code)) {
		return true;
	}

	switch (key_code)
	{
	case WXK_ESCAPE:
		m_canvas->GetCamera()->Reset();
		break;
	case WXK_SPACE:
		{
			m_selection.Traverse([](const ee0::GameObjWithPos& nwp)->bool
			{
#ifndef GAME_OBJ_ECS
				auto& ctrans = nwp.GetNode()->GetUniqueComp<n3::CompTransform>();
				ctrans.SetPosition(sm::vec3(0, 0, 0));
				ctrans.SetAngle(sm::Quaternion());
				ctrans.SetScale(sm::vec3(1, 1, 1));
#endif // GAME_OBJ_ECS
				return true;
			});
			m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
		}
		break;
	}

	return false;
}

bool NodeArrangeOP::OnMouseLeftDown(int x, int y)
{
	if (NodeSelectOP::OnMouseLeftDown(x, y)) {
		return true;
	}

	m_last_left_press.Set(x, y);

	auto& selection = m_stage.GetSelection();
	if (selection.IsEmpty()) {
		ChangeEditOpState(m_cam_rotate_state);
	} else {
		ChangeEditOpState(m_node_translate_state);
	}

	return m_op_state->OnMousePress(x, y);
}

bool NodeArrangeOP::OnMouseLeftUp(int x, int y)
{
	if (!m_last_left_press.IsValid()) {
		return false;
	}

	if (NodeSelectOP::OnMouseLeftUp(x, y)) {
		return true;
	}

	m_op_state->OnMouseRelease(x, y);

	ChangeEditOpState(m_cam_zoom_state);

	m_last_left_press.MakeInvalid();

	return false;
}

bool NodeArrangeOP::OnMouseRightDown(int x, int y)
{
	if (NodeSelectOP::OnMouseRightDown(x, y)) {
		return true;
	}

	m_last_right_press.Set(x, y);

	auto& selection = m_stage.GetSelection();
	if (selection.IsEmpty()) {
		ChangeEditOpState(m_cam_translate_state);
	} else if (selection.Size() == 1) {
		ChangeEditOpState(m_node_rotate_state);
	}

	return m_op_state->OnMousePress(x, y);
}

bool NodeArrangeOP::OnMouseRightUp(int x, int y)
{
	if (!m_last_right_press.IsValid()) {
		return false;
	}

	if (NodeSelectOP::OnMouseRightUp(x, y)) {
		return true;
	}

	m_op_state->OnMouseRelease(x, y);

	ChangeEditOpState(m_cam_zoom_state);

	m_last_right_press.MakeInvalid();

	return false;
}

bool NodeArrangeOP::OnMouseMove(int x, int y)
{
	if (NodeSelectOP::OnMouseMove(x, y)) {
		return true;
	}

	//m_stage->SetFocus();

	return m_op_state->OnMouseMove(x, y);
}

bool NodeArrangeOP::OnMouseDrag(int x, int y)
{
	if (!m_last_left_press.IsValid() &&
		!m_last_right_press.IsValid()) {
		return false;
	}

	if (NodeSelectOP::OnMouseDrag(x, y)) {
		return true;
	}

	return m_op_state->OnMouseDrag(x, y);
}

bool NodeArrangeOP::OnMouseWheelRotation(int x, int y, int direction)
{
	if (NodeSelectOP::OnMouseWheelRotation(x, y, direction)) {
		return true;
	}

	return m_op_state->OnMouseWheelRotation(x, y, direction);
}

}