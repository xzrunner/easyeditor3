#include "ee3/SkeletonIKOP.h"

#include <ee0/SubjectMgr.h>
#include <ee0/MessageID.h>
#include <ee3/MsgHelper.h>

#include <SM_Calc.h>
#include <SM_Ray.h>
#include <SM_RayIntersect.h>
#include <model/Model.h>
#include <model/ModelInstance.h>
#include <tessellation/Painter.h>
#include <painting2/OrthoCamera.h>
#include <painting2/RenderSystem.h>
#include <painting3/Viewport.h>
#include <painting3/PerspCam.h>

#include <array>

#define DEBUG_DRAW

namespace
{

static const float NODE_DRAW_RADIUS  = 5;
static const float NODE_QUERY_RADIUS = 10;

#ifdef DEBUG_DRAW
std::array<sm::vec3, 3> debug_pos;
void DebugDraw(const pt0::Camera& cam, const pt3::Viewport& vp)
{
	tess::Painter pt;

	auto cam_mat = cam.GetViewMat() * cam.GetProjectionMat();
	auto trans3d = [&](const sm::vec3& pos3)->sm::vec2 {
		return vp.TransPosProj3ToProj2(pos3, cam_mat);
	};

	pt.AddPolygon3D(debug_pos.data(), debug_pos.size(), trans3d, 0xff00ffff);

	const float radius = 5.0f;
	pt.AddCircleFilled(trans3d(debug_pos[0]), radius, 0xff0000ff);
	pt.AddCircleFilled(trans3d(debug_pos[1]), radius, 0xff00ff00);
	pt.AddCircleFilled(trans3d(debug_pos[2]), radius, 0xffff0000);

	pt2::RenderSystem::DrawPainter(pt);
}
#endif // DEBUG_DRAW

}

namespace ee3
{

SkeletonIKOP::SkeletonIKOP(const std::shared_ptr<pt0::Camera>& camera,
	                       const pt3::Viewport& vp,
	                       const ee0::SubjectMgrPtr& sub_mgr)
	: SkeletonSelectOp(camera, vp, sub_mgr)
{
}

bool SkeletonIKOP::OnMouseLeftDown(int x, int y)
{
	m_active = true;

	int selected = QueryJointByPos(*m_camera, x, y);
	if (selected != m_selected) {
		m_selected = selected;
		m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

		ee3::MsgHelper::SelectSkeletalJoint(*m_sub_mgr, m_selected);
	}

	if (m_selected < 0) {
		if (ee0::EditOP::OnMouseLeftDown(x, y)) {
			return true;
		}
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

	int selecting = QueryJointByPos(*m_camera, x, y);
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

	if (OneBone(x, y)) {
//	if (TwoBones(x, y)) {
		return true;
	}

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

	SkeletonSelectOp::OnDraw();

#ifdef DEBUG_DRAW
	DebugDraw(*m_camera, m_vp);
#endif // DEBUG_DRAW

	return false;
}

void SkeletonIKOP::OnModelChanged()
{
	InitTPoseTrans();
}

void SkeletonIKOP::InitTPoseTrans()
{
	// calc src global trans
	auto anim = static_cast<::model::SkeletalAnim*>(m_model->GetModel()->ext.get());
	auto& bones = anim->GetAllNodes();

	assert(bones.size() == m_model->GetLocalTrans().size());

	// to T-pose
	m_tpose_local_trans.resize(bones.size());
	for (size_t i = 0; i < bones.size(); ++i)
	{
		sm::vec3 pos, rot, scale;
		bones[i]->local_trans.Decompose(pos, rot, scale);

		auto& d = m_tpose_local_trans[i];
		d.c[0][0] = scale.x; d.c[0][1] = 0;       d.c[0][2] = 0;
		d.c[1][0] = 0;       d.c[1][1] = scale.y; d.c[1][2] = 0;
		d.c[2][0] = 0;       d.c[2][1] = 0;       d.c[2][2] = scale.z;
		d.c[3][0] = pos.x;   d.c[3][1] = pos.y;   d.c[3][2] = pos.z;
	}

	m_tpose_world_trans.resize(bones.size());
	for (size_t i = 0; i < bones.size(); ++i)
	{
		auto g_trans = m_tpose_local_trans[i];
		int parent = bones[i]->parent;
		while (parent != -1) {
			g_trans = g_trans * m_tpose_local_trans[parent];
			parent = bones[parent]->parent;
		}
		m_tpose_world_trans[i] = g_trans;
	}
}

// c_world = c_local * p_world;
// c_world = c_local * (sm::mat4(p_rot_delta) * p_local * pp_world);

bool SkeletonIKOP::OneBone(int x, int y)
{
	if (m_selected < 0) {
		return false;
	}

	auto& bones = (static_cast<::model::SkeletalAnim*>(m_model->GetModel()->ext.get())->GetAllNodes());
	int p = bones[m_selected]->parent;
	if (p < 0) {
		return false;
	}
	int pp = bones[p]->parent;
	if (pp < 0) {
		return false;
	}

	auto p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);
	sm::vec3 ray_dir = m_vp.TransPos3ScreenToDir(
		sm::vec2(static_cast<float>(x), static_cast<float>(y)), *p_cam);
	sm::Ray ray(p_cam->GetPos(), ray_dir);

	auto& g_trans = m_model->GetGlobalTrans();
	auto& l_trans = m_model->GetLocalTrans();
	auto c_pos = g_trans[m_selected] * sm::vec3(0, 0, 0);
	auto p_pos = g_trans[p] * sm::vec3(0, 0, 0);

	auto plane = GetRotatePlane(m_tpose_world_trans[p], p_pos);
	sm::vec3 cross;
	if (!sm::ray_plane_no_dir_intersect(ray, plane, &cross)) {
		return false;
	}

	sm::mat4 pp_inv;
	pp_inv = g_trans[pp];
	pp_inv.x[12] = pp_inv.x[13] = pp_inv.x[14] = 0;
	auto x2 = pp_inv * sm::vec3(1, 0, 0);
	auto y2 = pp_inv * sm::vec3(0, 1, 0);
	auto z2 = pp_inv * sm::vec3(0, 0, 1);
	pp_inv = pp_inv.Inverted();

	auto u = (m_tpose_world_trans[m_selected] * sm::vec3(0, 0, 0) - m_tpose_world_trans[p] * sm::vec3(0, 0, 0)).Normalized();
	auto v = (pp_inv * (cross - p_pos)).Normalized();

#ifdef DEBUG_DRAW
	debug_pos[0] = p_pos;
	debug_pos[1] = p_pos + u;
	debug_pos[2] = p_pos + v;
#endif // DEBUG_DRAW

	m_model->SetJointRotate(p, sm::Quaternion::CreateFromVectors(u, v));

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

#ifdef DEBUG_DRAW
	debug_pos[0] = pp_pos;
	debug_pos[1] = pp_other;
	debug_pos[2] = cross;
#endif // DEBUG_DRAW

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