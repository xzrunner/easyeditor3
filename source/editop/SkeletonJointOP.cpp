#include "ee3/SkeletonJointOP.h"
#include "ee3/RotateAxisState.h"
#include "ee3/TranslateAxisState.h"

#include <ee0/SubjectMgr.h>
#include <ee0/MessageID.h>
#include <ee3/MsgHelper.h>

#include <model/ModelInstance.h>
#include <painting3/PerspCam.h>

namespace ee3
{

SkeletonJointOP::SkeletonJointOP(const ur2::Device& dev, ur2::Context& ctx,
                                 const std::shared_ptr<pt0::Camera>& camera,
	                             const pt3::Viewport& vp,
	                             const ee0::SubjectMgrPtr& sub_mgr)
	: SkeletonSelectOp(dev, ctx, camera, vp, sub_mgr)
{
	InitRotateState(dev, ctx);
	InitTranslateState(dev, ctx);

	ChangeEditOpState(m_rotate_state);
}

bool SkeletonJointOP::OnMouseLeftDown(int x, int y)
{
	if (m_op_state->OnMousePress(x, y))
	{
		return true;
	}
	else
	{
		if (ee0::EditOP::OnMouseLeftDown(x, y)) {
			return true;
		}

		int selected = QueryJointByPos(*m_camera, x, y);
		if (selected >= 0 && selected != m_selected)
		{
			m_op_state->OnActive(true);
			m_selected = selected;
			m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

			ee3::MsgHelper::SelectSkeletalJoint(*m_sub_mgr, m_selected);
		}
	}

	return false;
}

bool SkeletonJointOP::OnMouseLeftUp(int x, int y)
{
	if (ee0::EditOP::OnMouseLeftUp(x, y)) {
		return true;
	}
	if (m_op_state->OnMousePress(x, y)) {
		return true;
	}

	return false;
}

bool SkeletonJointOP::OnMouseMove(int x, int y)
{
	if (ee0::EditOP::OnMouseMove(x, y)) {
		return true;
	}

	int selecting = QueryJointByPos(*m_camera, x, y);
	if (m_selecting != selecting) {
		m_selecting = selecting;
		m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
	}

	return false;
}

bool SkeletonJointOP::OnMouseDrag(int x, int y)
{
	if (ee0::EditOP::OnMouseDrag(x, y)) {
		return true;
	}
	if (m_op_state->OnMouseDrag(x, y)) {
		return true;
	}

	return false;
}

bool SkeletonJointOP::OnActive()
{
	if (ee0::EditOP::OnActive()) {
		return true;
	}
	if (m_op_state->OnActive(true)) {
		return true;
	}

	return false;
}

bool SkeletonJointOP::OnDraw(const ur2::Device& dev, ur2::Context& ctx) const
{
	if (ee0::EditOP::OnDraw(dev, ctx)) {
		return true;
	}

	SkeletonSelectOp::OnDraw(dev, ctx);

	if (m_op_state->OnDraw(dev, ctx)) {
		return true;
	}

	return false;
}

void SkeletonJointOP::ChangeToOpRotate()
{
	ChangeEditOpState(m_rotate_state);
}

void SkeletonJointOP::ChangeToOpTranslate()
{
	ChangeEditOpState(m_translate_state);
}

void SkeletonJointOP::InitRotateState(const ur2::Device& dev, ur2::Context& ctx)
{
	RotateAxisState::Callback cb;
	cb.is_need_draw = [&]() {
		return m_selected >= 0;
	};
	cb.get_origin_wmat = [&]()->sm::mat4 {
		if (m_model && m_selecting >= 0)
		{
			auto& g_trans = m_model->GetGlobalTrans();
			return g_trans[m_selecting];
		}
		else
		{
			return sm::mat4();
		}
	};
	cb.rotate = [&](const sm::Quaternion& delta) {
		if (m_selected >= 0) {
			m_model->RotateJoint(m_selected, delta);
		}
	};

	assert(m_camera->TypeID() == pt0::GetCamTypeID<pt3::PerspCam>());
	auto p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);

	RotateAxisState::Config cfg;
	cfg.arc_radius = 0.5f;

	m_rotate_state = std::make_shared<RotateAxisState>(
		dev, ctx, p_cam, m_vp, m_sub_mgr, cb, cfg
    );
}

void SkeletonJointOP::InitTranslateState(const ur2::Device& dev, ur2::Context& ctx)
{
	TranslateAxisState::Callback cb;
	cb.is_need_draw = [&]() {
		return m_selected >= 0;
	};
	cb.get_origin_wmat = [&]()->sm::mat4 {
		if (m_model && m_selecting >= 0)
		{
			auto& g_trans = m_model->GetGlobalTrans();
			return g_trans[m_selecting];
		}
		else
		{
			return sm::mat4();
		}
	};
	cb.translate = [&](const sm::vec3& offset) {
		if (m_selected >= 0) {
			m_model->TranslateJoint(m_selected, offset);
		}
	};

	assert(m_camera->TypeID() == pt0::GetCamTypeID<pt3::PerspCam>());
	auto p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);

	TranslateAxisState::Config cfg;
	cfg.arc_radius = 0.5f;

	m_translate_state = std::make_shared<TranslateAxisState>(
        dev, ctx, p_cam, m_vp, m_sub_mgr, cb, cfg
    );
}

}