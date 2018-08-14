#include "ee3/TranslateAxisState.h"

#include <ee0/SubjectMgr.h>
#include <ee0/MessageID.h>

#include <SM_Cube.h>
#include <SM_Calc.h>
#include <SM_RayIntersect.h>
#include <painting2/OrthoCamera.h>
#include <painting2/PrimitiveDraw.h>
#include <painting3/Viewport.h>
#include <painting3/PrimitiveDraw.h>
#include <painting3/PerspCam.h>
#ifndef GAME_OBJ_ECS
#include <node0/SceneNode.h>
#include <node3/CompTransform.h>
#include <node3/CompAABB.h>
#endif // GAME_OBJ_ECS

namespace ee3
{

TranslateAxisState::TranslateAxisState(const std::shared_ptr<pt0::Camera>& camera,
	                                   const pt3::Viewport& vp,
	                                   const ee0::SubjectMgrPtr& sub_mgr,
	                                   const Callback& cb, 
	                                   const Config& cfg)
	: ee0::EditOpState(camera)
	, m_vp(vp)
	, m_sub_mgr(sub_mgr)
	, m_cb(cb)
	, m_cfg(cfg)
	, m_cam2d(std::make_shared<pt2::OrthoCamera>())
	, m_op_type(POINT_QUERY_NULL)
{
	m_last_pos2.MakeInvalid();
	m_last_pos3.MakeInvalid();

	m_cam2d->OnSize(m_vp.Width(), m_vp.Height());
}

bool TranslateAxisState::OnMousePress(int x, int y)
{
	m_op_type = PointQuery(x, y);
	if (m_op_type == POINT_QUERY_NULL) {
		return false;
	}

	AxisNodeType axis;
	switch (m_op_type)
	{
	case POINT_QUERY_X:
		axis = AXIS_X;
		break;
	case POINT_QUERY_Y:
		axis = AXIS_Y;
		break;
	case POINT_QUERY_Z:
		axis = AXIS_Z;
		break;
	}

	auto cam_mat = m_camera->GetModelViewMat() * m_camera->GetProjectionMat();
	m_last_pos2 = GetCtrlPos2D(cam_mat, axis);
	m_first_pos2 = m_last_pos2;

	m_last_pos3 = m_rot_mat * GetCtrlPos3D(axis) + m_center;

	m_first_pos2 = m_last_pos2;

	m_move_path3d.origin = m_last_pos3;
	m_move_path3d.dir = (GetCtrlPos3D(axis) - GetCtrlPos3D(AXIS_CENTER)).Normalized();

	auto next_pos2 = m_vp.TransPosProj3ToProj2(m_move_path3d.origin + m_move_path3d.dir, cam_mat);
	m_first_dir2 = (next_pos2 - m_first_pos2).Normalized();

	return true;
}

bool TranslateAxisState::OnMouseRelease(int x, int y)
{
	m_op_type = POINT_QUERY_NULL;

	return false;
}

bool TranslateAxisState::OnMouseDrag(int x, int y)
{
	if (m_op_type == POINT_QUERY_NULL) {
		return false;
	}

	auto pos = m_cam2d->TransPosScreenToProject(x, y,
		static_cast<int>(m_vp.Width()), static_cast<int>(m_vp.Height()));
	Translate(m_last_pos2, pos);
	m_last_pos2 = pos;

	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

	return false;
}

bool TranslateAxisState::OnActive(bool active)
{
	UpdateSelectionSetInfo();

	return false;
}

bool TranslateAxisState::OnDraw() const
{
	//if (m_selection.IsEmpty()) {
	//	return false;
	//}
	if (!m_cb.is_need_draw()) {
		return false;
	}

	// 3d
	DrawEdges();

	// 2d
	DrawNodes();

	return false;
}

TranslateAxisState::PointQueryType TranslateAxisState::PointQuery(int x, int y) const
{
	auto cam_mat = m_camera->GetModelViewMat() * m_camera->GetProjectionMat();

	auto proj2d = m_cam2d->TransPosScreenToProject(x, y,
		static_cast<int>(m_vp.Width()), static_cast<int>(m_vp.Height()));
	// x, red
	auto pos2d = GetCtrlPos2D(cam_mat, AXIS_X);
	if (sm::dis_pos_to_pos(pos2d, proj2d) < m_cfg.node_radius) {
		return POINT_QUERY_X;
	}
	// y, green
	pos2d = GetCtrlPos2D(cam_mat, AXIS_Y);
	if (sm::dis_pos_to_pos(pos2d, proj2d) < m_cfg.node_radius) {
		return POINT_QUERY_Y;
	}
	// z, blue
	pos2d = GetCtrlPos2D(cam_mat, AXIS_Z);
	if (sm::dis_pos_to_pos(pos2d, proj2d) < m_cfg.node_radius) {
		return POINT_QUERY_Z;
	}

	return POINT_QUERY_NULL;
}

void TranslateAxisState::UpdateSelectionSetInfo()
{
	//sm::cube tot_aabb;
	//m_selection.Traverse([&](const ee0::GameObjWithPos& opw)->bool
	//{
	//	auto aabb = opw.GetNode()->GetUniqueComp<n3::CompAABB>().GetAABB();
	//	auto trans_aabb = aabb.Cube();
	//	trans_aabb.Translate(aabb.Position());
	//	tot_aabb.Combine(trans_aabb);
	//	return false;
	//});

	//m_center = tot_aabb.Center();

	m_cb.get_origin_transform(m_center, m_rot_mat);

	//int count = 0;
	//m_center.Set(0, 0, 0);
	//m_selection.Traverse([&](const ee0::GameObjWithPos& opw)->bool
	//{
	//	++count;
	//	auto node = opw.GetNode();
	//	auto aabb = node->GetUniqueComp<n3::CompAABB>().GetAABB();
	//	auto& ctrans = node->GetUniqueComp<n3::CompTransform>();
	//	m_rot_mat = sm::mat4(ctrans.GetAngle());
	//	auto pos = ctrans.GetTransformMat() * aabb.Cube().Center();
	//	m_center += pos;
	//	return false;
	//});
	//m_center /= static_cast<float>(count);

	auto cam_mat = m_camera->GetModelViewMat() * m_camera->GetProjectionMat();
	m_center2d = m_vp.TransPosProj3ToProj2(m_center, cam_mat);
}

void TranslateAxisState::Translate(const sm::vec2& start, const sm::vec2& end)
{
	if (m_camera->TypeID() == pt0::GetCamTypeID<pt3::PerspCam>())
	{
		auto p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);

		auto fixed_end = m_first_pos2 + m_first_dir2 * ((end - m_first_pos2).Dot(m_first_dir2));
		auto screen_fixed_end = m_cam2d->TransPosProjectToScreen(fixed_end,
			static_cast<int>(m_vp.Width()), static_cast<int>(m_vp.Height()));

		sm::vec3 ray_dir = m_vp.TransPos3ScreenToDir(screen_fixed_end, *p_cam);
		sm::Ray ray(p_cam->GetPos(), ray_dir);

		sm::vec3 cross_face_pos = m_move_path3d.origin + m_move_path3d.dir.Cross(ray_dir);
		sm::Plane cross_face(m_move_path3d.origin, m_move_path3d.origin + m_move_path3d.dir, cross_face_pos);

		sm::vec3 offset;
		sm::vec3 cross;
		if (sm::ray_plane_intersect(ray, cross_face, &cross)) {
			offset = cross - m_last_pos3;
			m_last_pos3 = cross;
		}

		m_cb.translate(offset);

	//	m_selection.Traverse([&](const ee0::GameObjWithPos& nwp)->bool
	//	{
	//#ifndef GAME_OBJ_ECS
	//		auto& ctrans = nwp.GetNode()->GetUniqueComp<n3::CompTransform>();
	//		ctrans.Translate(offset);
	//#endif // GAME_OBJ_ECS
	//		return true;
	//	});

		m_center += offset;
	}
}

void TranslateAxisState::DrawEdges() const
{
	// axis
	pt3::PrimitiveDraw::SetColor(0xff0000ff);
	sm::vec3 pos_x = m_rot_mat * sm::vec3(m_cfg.arc_radius, 0, 0) + m_center;
	pt3::PrimitiveDraw::Line(m_center, pos_x);
	pt3::PrimitiveDraw::SetColor(0xff00ff00);
	sm::vec3 pos_y = m_rot_mat * sm::vec3(0, m_cfg.arc_radius, 0) + m_center;
	pt3::PrimitiveDraw::Line(m_center, pos_y);
	pt3::PrimitiveDraw::SetColor(0xffff0000);
	sm::vec3 pos_z = m_rot_mat * sm::vec3(0, 0, -m_cfg.arc_radius) + m_center;
	pt3::PrimitiveDraw::Line(m_center, pos_z);
}

void TranslateAxisState::DrawNodes() const
{
	auto cam_mat = m_camera->GetModelViewMat() * m_camera->GetProjectionMat();
	// x, red
	auto pos2d = GetCtrlPos2D(cam_mat, AXIS_X);
	pt2::PrimitiveDraw::SetColor(0xff0000ff);
	pt2::PrimitiveDraw::Circle(nullptr, pos2d, m_cfg.node_radius, true);
	// y, green
	pos2d = GetCtrlPos2D(cam_mat, AXIS_Y);
	pt2::PrimitiveDraw::SetColor(0xff00ff00);
	pt2::PrimitiveDraw::Circle(nullptr, pos2d, m_cfg.node_radius, true);
	// z, blue
	pos2d = GetCtrlPos2D(cam_mat, AXIS_Z);
	pt2::PrimitiveDraw::SetColor(0xffff0000);
	pt2::PrimitiveDraw::Circle(nullptr, pos2d, m_cfg.node_radius, true);
}

sm::vec2 TranslateAxisState::GetCtrlPos2D(const sm::mat4& cam_mat, AxisNodeType type) const
{
	return m_vp.TransPosProj3ToProj2(m_rot_mat * GetCtrlPos3D(type) + m_center, cam_mat);
}

sm::vec3 TranslateAxisState::GetCtrlPos3D(AxisNodeType type) const
{
	sm::vec3 pos;
	switch (type)
	{
	case AXIS_CENTER:
		pos.Set(0, 0, 0);
		break;
	case AXIS_X:
		pos.Set(m_cfg.arc_radius, 0, 0);
		break;
	case AXIS_Y:
		pos.Set(0, m_cfg.arc_radius, 0);
		break;
	case AXIS_Z:
		pos.Set(0, 0, -m_cfg.arc_radius);
		break;
	}
	return pos;
}

}