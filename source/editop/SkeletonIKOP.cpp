#include "ee3/SkeletonIKOP.h"

#include <ee0/SubjectMgr.h>
#include <ee0/MessageID.h>
#include <ee3/MsgHelper.h>

#include <SM_Calc.h>
#include <SM_Ray.h>
#include <SM_RayIntersect.h>
#include <model/Model.h>
#include <model/ModelInstance.h>
#include <model/AnimIK.h>
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

//	if (OneBone(x, y)) {
	if (TwoBones(x, y)) {
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

// c_world = c_local * p_world;
// c_world = c_local * (sm::mat4(p_rot_delta) * p_local * pp_world);

bool SkeletonIKOP::OneBone(int x, int y)
{
	if (m_selected < 0) {
		return false;
	}

	auto sk_anim = static_cast<::model::SkeletalAnim*>(m_model->GetModel()->ext.get());
	auto& bones = sk_anim->GetNodes();
	int p = bones[m_selected]->parent;
	if (p < 0) {
		return false;
	}

	auto p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);
	sm::vec3 ray_dir = m_vp.TransPos3ScreenToDir(
		sm::vec2(static_cast<float>(x), static_cast<float>(y)), *p_cam);
	sm::Ray ray(p_cam->GetPos(), ray_dir);

	auto& g_trans = m_model->GetGlobalTrans();
	auto c_pos = g_trans[m_selected] * sm::vec3(0, 0, 0);
	auto p_pos = g_trans[p] * sm::vec3(0, 0, 0);

	auto& tp_wtrans = sk_anim->GetTPWorldTrans();

	sm::vec3 rot_axis = (p_cam->GetPos() - c_pos).Normalized();
	auto plane = GetRotatePlane(tp_wtrans[p], p_pos, rot_axis);
	sm::vec3 cross;
	if (!sm::ray_plane_no_dir_intersect(ray, plane, &cross)) {
		return false;
	}

	::model::AnimIK::OneBone(*m_model, m_selected, cross, debug_pos);

	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

	return true;
}

bool SkeletonIKOP::TwoBones(int x, int y)
{
	if (m_selected < 0) {
		return false;
	}

	auto sk_anim = static_cast<::model::SkeletalAnim*>(m_model->GetModel()->ext.get());
	auto& bones = sk_anim->GetNodes();
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
	auto c_pos   = g_trans[m_selected] * sm::vec3(0, 0, 0);
	auto pp_pos  = g_trans[pp]         * sm::vec3(0, 0, 0);

	auto& tp_wtrans = sk_anim->GetTPWorldTrans();

	sm::vec3 rot_axis = (p_cam->GetPos() - c_pos).Normalized();
	auto plane = GetRotatePlane(tp_wtrans[pp], pp_pos, rot_axis);
	sm::vec3 cross;
	if (!sm::ray_plane_no_dir_intersect(ray, plane, &cross)) {
		return false;
	}

//	::model::AnimIK::TwoBones(*m_model, m_selected, cross, rot_axis, debug_pos);
	::model::AnimIK::TwoBones(*m_model, m_selected, cross, sm::vec3(0, 1, 0), debug_pos);

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