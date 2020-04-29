#include "ee3/NodeArrangeOP.h"
#include "ee3/WxStageCanvas.h"
#include "ee3/CamTranslateState.h"
#include "ee3/CamRotateState.h"
#include "ee3/CamZoomState.h"
#include "ee3/NodeTranslateState.h"
#include "ee3/TranslateAxisState.h"

#include <ee0/WxStagePage.h>
#include <ee0/SubjectMgr.h>

#include <painting3/PerspCam.h>
#ifndef GAME_OBJ_ECS
#include <node0/SceneNode.h>
#include <node3/CompTransform.h>
#include <node3/CompAABB.h>
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

    m_ops[OP_CAM_ZOOM] = std::make_shared<CamZoomState>(p_cam, vp, m_sub_mgr);

    m_ops[OP_CAM_ROTATE] = std::make_shared<CamRotateState>(p_cam, m_sub_mgr);
    m_ops[OP_CAM_ROTATE]->SetPrevOpState(m_ops[OP_CAM_ZOOM]);
    m_ops[OP_CAM_TRANSLATE] = std::make_shared<CamTranslateState>(p_cam, m_sub_mgr);
    m_ops[OP_CAM_TRANSLATE]->SetPrevOpState(m_ops[OP_CAM_ZOOM]);

    TranslateAxisState::Callback cb;
    cb.is_need_draw = [&]() {
        return !stage.GetSelection().IsEmpty();
    };
    cb.get_origin_wmat = [&]()->sm::mat4 {
        sm::vec3 center;
        sm::Quaternion angle;
        int count = 0;
        stage.GetSelection().Traverse([&](const ee0::GameObjWithPos& opw)->bool
        {
            ++count;
            auto node = opw.GetNode();
            auto aabb = opw.GetNode()->GetUniqueComp<n3::CompAABB>().GetAABB();
            auto& ctrans = opw.GetNode()->GetUniqueComp<n3::CompTransform>();
            angle = ctrans.GetAngle();
            auto pos = ctrans.GetTransformMat() * aabb.Cube().Center();
            center += pos;
            return false;
        });
        center /= static_cast<float>(count);
        auto trans_mat = sm::mat4::Translated(center.x, center.y, center.z);
        auto rot_mat = sm::mat4(angle);
        return trans_mat * rot_mat;
    };
    cb.translate = [&](const sm::vec3& offset) {
        m_selection.Traverse([&](const ee0::GameObjWithPos& nwp)->bool
        {
            auto& node = nwp.GetNode();
            auto& ctrans = node->GetUniqueComp<n3::CompTransform>();
            ctrans.Translate(offset);
            return true;
        });
    };

    auto canvas = m_stage.GetImpl().GetCanvas();
    auto& dev = canvas->GetRenderDevice();
    auto& ctx = *canvas->GetRenderContext().ur_ctx;
    m_ops[OP_NODE_TRANSLATE] = std::make_shared<TranslateAxisState>(
        dev, ctx, p_cam, vp, m_sub_mgr, cb, TranslateAxisState::Config(0.5f, 5));
    m_ops[OP_NODE_TRANSLATE]->SetPrevOpState(m_ops[OP_CAM_ZOOM]);

	m_op_state = m_ops[OP_CAM_ROTATE];

	m_last_left_press.MakeInvalid();
    m_last_middle_press.MakeInvalid();
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
    auto ret = m_op_state->OnMousePress(x, y);

	if (!ret && NodeSelectOP::OnMouseLeftDown(x, y)) {
		return true;
	}

	m_last_left_press.Set(x, y);

	auto& selection = m_stage.GetSelection();
	if (selection.IsEmpty()) {
		ChangeEditOpState(m_ops[OP_CAM_ROTATE]);
	} else {
		ChangeEditOpState(m_ops[OP_NODE_TRANSLATE]);
	}

    return ret;
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

	m_last_left_press.MakeInvalid();

	return false;
}

bool NodeArrangeOP::OnMouseMiddleDown(int x, int y)
{
	if (NodeSelectOP::OnMouseMiddleDown(x, y)) {
		return true;
	}

    m_last_middle_press.Set(x, y);

    ChangeEditOpState(m_ops[OP_CAM_TRANSLATE]);

    return m_op_state->OnMousePress(x, y);
}

bool NodeArrangeOP::OnMouseMiddleUp(int x, int y)
{
    if (!m_last_middle_press.IsValid()) {
        return false;
    }

	if (NodeSelectOP::OnMouseMiddleUp(x, y)) {
		return true;
	}

	m_op_state->OnMouseRelease(x, y);

    m_last_middle_press.MakeInvalid();

	return false;
}

bool NodeArrangeOP::OnMouseRightDown(int x, int y)
{
	if (NodeSelectOP::OnMouseRightDown(x, y)) {
		return true;
	}

	m_last_right_press.Set(x, y);

    ChangeEditOpState(m_ops[OP_CAM_ROTATE]);

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
        !m_last_middle_press.IsValid() &&
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

bool NodeArrangeOP::OnDraw(const ur::Device& dev, ur::Context& ctx) const
{
    if (NodeSelectOP::OnDraw(dev, ctx)) {
        return true;
    }

    return m_ops[OP_NODE_TRANSLATE]->OnDraw(dev, ctx);
}

void NodeArrangeOP::SetCamera(const std::shared_ptr<pt0::Camera>& camera)
{
    NodeSelectOP::SetCamera(camera);

    for (auto& op : m_ops) {
        op->SetCamera(camera);
    }
}

void NodeArrangeOP::AfterInsertSelected(const n0::SceneNodePtr& node) const
{
    NodeSelectOP::AfterInsertSelected(node);

    // update edited node's coordinate system
    m_ops[OP_NODE_TRANSLATE]->OnActive(true);
}

void NodeArrangeOP::AfterDeleteSelected(const n0::SceneNodePtr& node) const
{
    NodeSelectOP::AfterDeleteSelected(node);

    // update edited node's coordinate system
    m_ops[OP_NODE_TRANSLATE]->OnActive(false);
}

void NodeArrangeOP::AfterClearSelection() const
{
    NodeSelectOP::AfterClearSelection();

    // update edited node's coordinate system
    m_ops[OP_NODE_TRANSLATE]->OnActive(false);
}

}