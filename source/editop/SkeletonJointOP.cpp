#include "ee3/SkeletonJointOP.h"
#include "ee3/RotateAxisState.h"
#include "ee3/TranslateAxisState.h"

#include <ee0/SubjectMgr.h>
#include <ee0/MessageID.h>

#include <SM_Calc.h>
#include <model/Model.h>
#include <model/ModelInstance.h>
#include <painting2/PrimitiveDraw.h>
#include <painting2/OrthoCamera.h>
#include <painting3/Viewport.h>
#include <painting3/PrimitiveDraw.h>
#include <painting3/PerspCam.h>

namespace
{

static const float NODE_DRAW_RADIUS  = 5;
static const float NODE_QUERY_RADIUS = 10;

}

namespace ee3
{

SkeletonJointOP::SkeletonJointOP(const std::shared_ptr<pt0::Camera>& camera,
	                           const pt3::Viewport& vp,
	                           const ee0::SubjectMgrPtr& sub_mgr)
	: ee0::EditOP(camera)
	, m_vp(vp)
	, m_sub_mgr(sub_mgr)
	, m_cam2d(std::make_shared<pt2::OrthoCamera>())
{
	InitRotateState();
	InitTranslateState();

	ChangeEditOpState(m_rotate_state);
}

bool SkeletonJointOP::OnMouseLeftDown(int x, int y)
{
	if (ee0::EditOP::OnMouseLeftDown(x, y)) {
		return true;
	}

	if (m_op_state->OnMousePress(x, y))
	{
		return true;
	}
	else
	{
		int selected = QueryJointByPos(x, y);
		m_op_state->OnActive(selected > 0);
		if (selected != m_selected) {
			m_selected = selected;
			m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
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

	int selecting = QueryJointByPos(x, y);
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

bool SkeletonJointOP::OnDraw() const
{
	if (ee0::EditOP::OnDraw()) {
		return true;
	}

	if (!m_model) {
		return false;
	}

	auto& bones = (static_cast<::model::SkeletalAnim*>(m_model->GetModel()->ext.get())->GetAllNodes());

	auto& g_trans = m_model->GetGlobalTrans();
	for (int i = 0, n = bones.size(); i < n; ++i)
	{
		// point
		auto p_pos = g_trans[i] * sm::vec3(0, 0, 0);
		if (i == m_selecting) {
			// FIXME: set color will no use here, if not flush shader by call PointSize()
			pt2::PrimitiveDraw::PointSize(NODE_DRAW_RADIUS * 2);
			pt3::PrimitiveDraw::SetColor(0xff00ffff);
		} else {
			pt2::PrimitiveDraw::PointSize(NODE_DRAW_RADIUS);
			pt3::PrimitiveDraw::SetColor(0xffffff00);
		}
		pt3::PrimitiveDraw::Point(p_pos);

		// edge
		pt2::PrimitiveDraw::LineWidth(1);
		for (auto& child : bones[i]->children)
		{
			auto c_pos = g_trans[child] * sm::vec3(0, 0, 0);
			pt3::PrimitiveDraw::SetColor(0xffff00ff);
			pt3::PrimitiveDraw::Line(p_pos, c_pos);
		}
	}

	if (m_op_state->OnDraw()) {
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

void SkeletonJointOP::InitRotateState()
{
	RotateAxisState::Callback cb;
	cb.is_need_draw = [&]() {
		return m_selected >= 0;
	};
	cb.get_origin_transform = [&](sm::vec3& pos, sm::mat4& mat) {
		if (m_model && m_selecting >= 0)
		{
			auto& g_trans = m_model->GetGlobalTrans();
			pos = g_trans[m_selecting] * sm::vec3(0, 0, 0);
			mat = g_trans[m_selecting];
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
		p_cam, m_vp, m_sub_mgr, cb, cfg);
}

void SkeletonJointOP::InitTranslateState()
{
	TranslateAxisState::Callback cb;
	cb.is_need_draw = [&]() {
		return m_selected >= 0;
	};
	cb.get_origin_transform = [&](sm::vec3& pos, sm::mat4& rot_mat) {
		if (m_model && m_selecting >= 0)
		{
			auto& g_trans = m_model->GetGlobalTrans();
			pos = g_trans[m_selecting] * sm::vec3(0, 0, 0);
			rot_mat = g_trans[m_selecting];
			rot_mat.x[12] = rot_mat.x[13] = rot_mat.x[14] = 0;
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
		p_cam, m_vp, m_sub_mgr, cb, cfg);
}

int SkeletonJointOP::QueryJointByPos(int x, int y) const
{
	if (!m_model) {
		return -1;
	}

	auto pos = m_cam2d->TransPosScreenToProject(x, y,
		static_cast<int>(m_vp.Width()), static_cast<int>(m_vp.Height()));
	auto cam_mat = m_camera->GetModelViewMat() * m_camera->GetProjectionMat();

	auto& g_trans = m_model->GetGlobalTrans();
	auto& bones = (static_cast<::model::SkeletalAnim*>(m_model->GetModel()->ext.get())->GetAllNodes());
	for (int i = 0, n = bones.size(); i < n; ++i)
	{
		auto b_pos = g_trans[i] * sm::vec3(0, 0, 0);
		auto p = m_vp.TransPosProj3ToProj2(b_pos, cam_mat);
		if (sm::dis_pos_to_pos(p, pos) < NODE_QUERY_RADIUS) {
			return i;
		}
	}
	return -1;
}

}