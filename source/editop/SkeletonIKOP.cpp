#include "ee3/SkeletonIKOP.h"

#include <ee0/SubjectMgr.h>
#include <ee0/MessageID.h>

#include <SM_Calc.h>
#include <SM_Ray.h>
#include <SM_RayIntersect.h>
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

	m_active = true;

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

	m_active = false;

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

	if (!m_active) {
		return false;
	}

//	OneBone(x, y);
	TwoBones(x, y);

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

	//// debug draw
	//pt2::PrimitiveDraw::LineWidth(3);
	//pt3::PrimitiveDraw::SetColor(0xff0000ff);
	//pt3::PrimitiveDraw::TriLine(m_debug[0], m_debug[1], m_debug[2]);

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

// c_world = c_local * p_world;
// c_world = c_local * (sm::mat4(p_rot_delta) * p_local * pp_world);

bool SkeletonIKOP::OneBone(int x, int y)
{
	if (m_selected < 0) {
		return false;
	}

	auto& bones = (static_cast<::model::SkeletalAnim*>(m_model->GetModel()->ext.get())->GetAllNodes());
	int parent = bones[m_selected]->parent;
	if (parent < 0) {
		return false;
	}
	int grandparent = bones[parent]->parent;
	if (grandparent < 0) {
		return false;
	}

	auto p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);
	sm::vec3 ray_dir = m_vp.TransPos3ScreenToDir(
		sm::vec2(static_cast<float>(x), static_cast<float>(y)), *p_cam);
	sm::Ray ray(p_cam->GetPos(), ray_dir);

	auto& g_trans = m_model->GetGlobalTrans();
	auto& l_trans = m_model->GetLocalTrans();
	auto c_pos = g_trans[m_selected] * sm::vec3(0, 0, 0);
	auto p_pos = g_trans[parent] * sm::vec3(0, 0, 0);

	auto plane = GetRotatePlane(g_trans[parent], p_pos);
	sm::vec3 cross;
	if (!sm::ray_plane_no_dir_intersect(ray, plane, &cross)) {
		return false;
	}

	int pparent = bones[parent]->parent;
	if (pparent < 0) {
		return false;
	}
	auto rot_pp = g_trans[pparent];
	rot_pp.x[12] = rot_pp.x[13] = rot_pp.x[14] = 0;
	auto p_other = p_pos + rot_pp * sm::vec3(1, 0, 0);

	// debug draw
	m_debug[0] = p_pos;
	m_debug[1] = p_other;
	m_debug[2] = cross;

	float angle = GetRotateAngle(g_trans[parent], p_pos, p_other, cross);
	m_model->SetJointRotate(parent, bones[parent]->local_trans, sm::Quaternion::CreateFromEulerAngle(-angle, 0, 0));

	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

	return true;
}

bool SkeletonIKOP::TwoBones(int x, int y)
{
	if (m_selected < 0) {
		return false;
	}

	auto& bones = (static_cast<::model::SkeletalAnim*>(m_model->GetModel()->ext.get())->GetAllNodes());
	int parent = bones[m_selected]->parent;
	if (parent < 0) {
		return false;
	}
	int pparent = bones[parent]->parent;
	if (pparent < 0) {
		return false;
	}
	int ppparent = bones[pparent]->parent;
	if (ppparent < 0) {
		return false;
	}

	auto p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);
	sm::vec3 ray_dir = m_vp.TransPos3ScreenToDir(
		sm::vec2(static_cast<float>(x), static_cast<float>(y)), *p_cam);
	sm::Ray ray(p_cam->GetPos(), ray_dir);

	auto& g_trans = m_model->GetGlobalTrans();
	auto c_pos   = g_trans[m_selected] * sm::vec3(0, 0, 0);
	auto p_pos   = g_trans[parent]     * sm::vec3(0, 0, 0);
	auto pp_pos  = g_trans[pparent]    * sm::vec3(0, 0, 0);
	auto ppp_pos = g_trans[ppparent]   * sm::vec3(0, 0, 0);

	//auto plane = GetRotatePlane(g_trans[parent], p_pos);
	//sm::vec3 cross;
	//if (!sm::ray_plane_no_dir_intersect(ray, plane, &cross)) {
	//	return false;
	//}

	auto plane = GetRotatePlane(g_trans[pparent], pp_pos);
	sm::vec3 cross;
	if (!sm::ray_plane_no_dir_intersect(ray, plane, &cross)) {
		return false;
	}

	auto rot_ppp = g_trans[ppparent];
	rot_ppp.x[12] = rot_ppp.x[13] = rot_ppp.x[14] = 0;
	auto pp_other = pp_pos + rot_ppp * sm::vec3(1, 0, 0);

	// debug draw
	m_debug[0] = pp_pos;
	m_debug[1] = pp_other;
	m_debug[2] = cross;

	float angle = GetRotateAngle(g_trans[pparent], pp_pos, pp_other, cross);

	float len0 = sm::dis_pos3_to_pos3(pp_pos, p_pos);
	float len1 = sm::dis_pos3_to_pos3(p_pos, c_pos);
	float tot_len = sm::dis_pos3_to_pos3(cross, pp_pos);
	if (tot_len < len0 + len1 && len0 < tot_len + len1 && len1 < tot_len + len0)
	{
		float ang0 = acosf((len0 * len0 + tot_len * tot_len - len1 * len1) / (2 * len0 * tot_len));
		float ang1 = acosf((len1 * len1 + tot_len * tot_len - len0 * len0) / (2 * len1 * tot_len));
		m_model->SetJointRotate(pparent, bones[pparent]->local_trans, sm::Quaternion::CreateFromEulerAngle(-(angle - ang0), 0, 0));
		m_model->SetJointRotate(parent, bones[parent]->local_trans, sm::Quaternion::CreateFromEulerAngle(-(ang0 + ang1), 0, 0));
	}
	else
	{
		m_model->SetJointRotate(pparent, bones[pparent]->local_trans, sm::Quaternion::CreateFromEulerAngle(-angle, 0, 0));
		m_model->SetJointRotate(parent, bones[parent]->local_trans, sm::Quaternion());
	}

	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

	return true;
}

sm::Plane SkeletonIKOP::GetRotatePlane(const sm::mat4& world_mat, const sm::vec3& pos,
	                                   const sm::vec3& rot_axis)
{
	auto rot_mat = world_mat;
	rot_mat.x[12] = rot_mat.x[13] = rot_mat.x[14] = 0;
	auto normal = rot_mat * rot_axis;
	return sm::Plane(normal, pos);
}

float SkeletonIKOP::GetRotateAngle(const sm::mat4& world_mat, const sm::vec3& base,
	                               const sm::vec3& from, const sm::vec3& to)
{
	auto rot_mat = world_mat;
	rot_mat.x[12] = rot_mat.x[13] = rot_mat.x[14] = 0;

	auto v0 = from - base;
	auto v1 = to - base;
	float v_cos = v0.Dot(v1) / (v0.Length() * v1.Length());
	v_cos = std::max(std::min(v_cos, 1.0f), -1.0f);

	float angle = acosf(v_cos);
	auto dir = v0.Cross(v1).Dot(rot_mat * sm::vec3(0, 0, -1));
	if (dir > 0) {
		angle = fabs(angle);
	} else {
		angle = -fabs(angle);
	}
	return angle;
}

}