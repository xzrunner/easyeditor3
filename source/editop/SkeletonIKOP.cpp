#include "ee3/SkeletonIKOP.h"

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

SkeletonIKOP::SkeletonIKOP(const std::shared_ptr<pt0::Camera>& camera,
	                           const pt3::Viewport& vp,
	                           const ee0::SubjectMgrPtr& sub_mgr)
	: ee0::EditOP(camera)
	, m_vp(vp)
	, m_sub_mgr(sub_mgr)
	, m_cam2d(std::make_shared<pt2::OrthoCamera>())
{
}

bool SkeletonIKOP::OnMouseLeftDown(int x, int y)
{
	if (ee0::EditOP::OnMouseLeftDown(x, y)) {
		return true;
	}

	int selected = QueryJointByPos(x, y);
	if (selected != m_selected) {
		m_selected = selected;
		m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
	}

	return false;
}

bool SkeletonIKOP::OnMouseLeftUp(int x, int y)
{
	if (ee0::EditOP::OnMouseLeftUp(x, y)) {
		return true;
	}

	return false;
}

bool SkeletonIKOP::OnMouseMove(int x, int y)
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

bool SkeletonIKOP::OnMouseDrag(int x, int y)
{
	if (ee0::EditOP::OnMouseDrag(x, y)) {
		return true;
	}
	if (m_selected < 0) {
		return false;
	}

	auto cam_mat = m_camera->GetModelViewMat() * m_camera->GetProjectionMat();
	auto& g_trans = m_model->GetGlobalTrans();
	auto& bones = (static_cast<::model::SkeletalAnim*>(m_model->GetModel()->ext.get())->GetAllNodes());
	assert(m_selected < g_trans.size());
	auto b_pos = g_trans[m_selected] * sm::vec3(0, 0, 0);
	auto src_pos = m_vp.TransPosProj3ToProj2(b_pos, cam_mat);

	auto dst_pos = m_cam2d->TransPosScreenToProject(x, y,
		static_cast<int>(m_vp.Width()), static_cast<int>(m_vp.Height()));

	// calc
	int parent = bones[m_selected]->parent;
	if (parent < 0) {
		return false;
	}
	auto& l_trans = m_model->GetLocalTrans();
	auto& p_world = g_trans[parent];
	auto& pp_world = g_trans[bones[parent]->parent];
	auto& p_local = l_trans[parent];
	auto& c_local = l_trans[m_selected];

	float min_dist = sm::dis_square_pos_to_pos(src_pos, dst_pos);
	int   min_deg = 0;

	float angle = 0;
	const float d_angle = 1.0f;
	while (true) {
		angle += d_angle;
		auto delta = sm::Quaternion::CreateFromEulerAngle(-angle * SM_DEG_TO_RAD, 0, 0);

		// c_world = c_local * p_world;
		auto c_world = c_local * (sm::mat4(delta) * p_local * pp_world);
		auto pos = m_vp.TransPosProj3ToProj2(c_world * sm::vec3(0, 0, 0), cam_mat);
		float d = sm::dis_square_pos_to_pos(pos, dst_pos);
		if (d < min_dist) {
			min_dist = d;
			min_deg = angle;
		} else {
			break;
		}
	}
	angle = 0;
	while (true) {
		angle -= d_angle;
		auto delta = sm::Quaternion::CreateFromEulerAngle(-angle * SM_DEG_TO_RAD, 0, 0);

		// c_world = c_local * p_world;
		auto c_world = c_local * (sm::mat4(delta) * p_local * pp_world);
		auto pos = m_vp.TransPosProj3ToProj2(c_world * sm::vec3(0, 0, 0), cam_mat);
		float d = sm::dis_square_pos_to_pos(pos, dst_pos);
		if (d < min_dist) {
			min_dist = d;
			min_deg = angle;
		} else {
			break;
		}
	}

	m_model->RotateJoint(parent, sm::Quaternion::CreateFromEulerAngle(-min_deg * SM_DEG_TO_RAD, 0, 0));

	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

	return false;
}

bool SkeletonIKOP::OnDraw() const
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

	return false;
}

int SkeletonIKOP::QueryJointByPos(int x, int y) const
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