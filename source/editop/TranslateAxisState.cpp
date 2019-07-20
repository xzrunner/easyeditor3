#include "ee3/TranslateAxisState.h"

#include <ee0/SubjectMgr.h>
#include <ee0/MessageID.h>

#include <SM_Cube.h>
#include <SM_Calc.h>
#include <SM_RayIntersect.h>
#include <painting2/OrthoCamera.h>
#include <painting2/RenderSystem.h>
#include <painting3/Viewport.h>
#include <painting3/PerspCam.h>
#ifndef GAME_OBJ_ECS
#include <node0/SceneNode.h>
#include <node3/CompTransform.h>
#include <node3/CompAABB.h>
#endif // GAME_OBJ_ECS
#include <tessellation/Painter.h>

namespace
{

const float TRANS_FACE_SIZE = 0.3f;

}

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
    sm::vec3 pos;
	m_op_type = PointQuery(x, y, pos);
	if (m_op_type == POINT_QUERY_NULL) {
		return false;
	}

	auto cam_mat = m_camera->GetProjectionMat() * m_camera->GetViewMat();
	m_last_pos2 = m_vp.TransPosProj3ToProj2(m_ori_wmat_no_scale * pos, cam_mat);
	m_first_pos2 = m_last_pos2;

	m_last_pos3 = m_ori_wmat_no_scale * pos;

	m_first_pos2 = m_last_pos2;

	m_move_path3d.origin = m_last_pos3;
	m_move_path3d.dir = (pos - GetCtrlPos3D(AXIS_CENTER)).Normalized();

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
	Translate(x, y);
	m_last_pos2 = pos;

	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

	return false;
}

bool TranslateAxisState::OnActive(bool active)
{
	if (active) {
		UpdateSelectionSetInfo();
	}

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

	tess::Painter pt;

	auto cam_mat = m_camera->GetProjectionMat() * m_camera->GetViewMat();

	const float line_width = 2.0f;

	auto c = GetCtrlPos2D(cam_mat, AXIS_CENTER);
	auto x = GetCtrlPos2D(cam_mat, AXIS_X);
	auto y = GetCtrlPos2D(cam_mat, AXIS_Y);
	auto z = GetCtrlPos2D(cam_mat, AXIS_Z);

	// draw edges
	// axis
	pt.AddLine(c, x, 0xff0000ff, line_width);
	pt.AddLine(c, y, 0xff00ff00, line_width);
	pt.AddLine(c, z, 0xffff0000, line_width);

	// draw nodes
	// x, red
	pt.AddCircleFilled(x, m_cfg.node_radius, 0xff0000ff);
	// y, green
	pt.AddCircleFilled(y, m_cfg.node_radius, 0xff00ff00);
	// z, blue
	pt.AddCircleFilled(z, m_cfg.node_radius, 0xffff0000);

    // draw face
    auto cc = GetCtrlPos3D(AXIS_CENTER) * TRANS_FACE_SIZE;
    auto cx = GetCtrlPos3D(AXIS_X) * TRANS_FACE_SIZE;
    auto cy = GetCtrlPos3D(AXIS_Y) * TRANS_FACE_SIZE;
    auto cz = GetCtrlPos3D(AXIS_Z) * TRANS_FACE_SIZE;
    auto cxy = cx + cy - cc;
    auto cyz = cy + cz - cc;
    auto czx = cz + cx - cc;
    auto cc2 = m_vp.TransPosProj3ToProj2(m_ori_wmat_no_scale * cc, cam_mat);
    auto cx2 = m_vp.TransPosProj3ToProj2(m_ori_wmat_no_scale * cx, cam_mat);
    auto cy2 = m_vp.TransPosProj3ToProj2(m_ori_wmat_no_scale * cy, cam_mat);
    auto cz2 = m_vp.TransPosProj3ToProj2(m_ori_wmat_no_scale * cz, cam_mat);
    auto cxy2 = m_vp.TransPosProj3ToProj2(m_ori_wmat_no_scale * cxy, cam_mat);
    auto cyz2 = m_vp.TransPosProj3ToProj2(m_ori_wmat_no_scale * cyz, cam_mat);
    auto czx2 = m_vp.TransPosProj3ToProj2(m_ori_wmat_no_scale * czx, cam_mat);

    const float size = 0.5f;
    sm::vec2 poly_xy[4] = { cc2, cx2, cxy2, cy2 };
    pt.AddPolygonFilled(poly_xy, 4, 0x88ff0000);
    sm::vec2 poly_yz[4] = { cc2, cy2, cyz2, cz2 };
    pt.AddPolygonFilled(poly_yz, 4, 0x880000ff);
    sm::vec2 poly_zx[4] = { cc2, cz2, czx2, cx2 };
    pt.AddPolygonFilled(poly_zx, 4, 0x8800ff00);

	pt2::RenderSystem::DrawPainter(pt);

	return false;
}

TranslateAxisState::PointQueryType TranslateAxisState::PointQuery(int x, int y, sm::vec3& pos) const
{
	auto cam_mat = m_camera->GetProjectionMat() * m_camera->GetViewMat();

    // point test

	auto proj2d = m_cam2d->TransPosScreenToProject(x, y,
		static_cast<int>(m_vp.Width()), static_cast<int>(m_vp.Height()));
	// x, red
	auto pos2d = GetCtrlPos2D(cam_mat, AXIS_X);
	if (sm::dis_pos_to_pos(pos2d, proj2d) < m_cfg.node_radius) {
        pos.Set(CalcCoordAxisLen(), 0, 0);
		return POINT_QUERY_X;
	}
	// y, green
	pos2d = GetCtrlPos2D(cam_mat, AXIS_Y);
	if (sm::dis_pos_to_pos(pos2d, proj2d) < m_cfg.node_radius) {
        pos.Set(0, CalcCoordAxisLen(), 0);
		return POINT_QUERY_Y;
	}
	// z, blue
	pos2d = GetCtrlPos2D(cam_mat, AXIS_Z);
	if (sm::dis_pos_to_pos(pos2d, proj2d) < m_cfg.node_radius) {
        pos.Set(0, 0, -CalcCoordAxisLen());
		return POINT_QUERY_Z;
	}

    // face test

    auto cc = GetCtrlPos3D(AXIS_CENTER) * TRANS_FACE_SIZE;
    auto cx = GetCtrlPos3D(AXIS_X) * TRANS_FACE_SIZE;
    auto cy = GetCtrlPos3D(AXIS_Y) * TRANS_FACE_SIZE;
    auto cz = GetCtrlPos3D(AXIS_Z) * TRANS_FACE_SIZE;
    auto cxy = cx + cy - cc;
    auto cyz = cy + cz - cc;
    auto czx = cz + cx - cc;

    sm::vec3 face_xy[4] = { cc, cx, cxy, cy };
    sm::vec3 face_yz[4] = { cc, cy, cyz, cz };
    sm::vec3 face_zx[4] = { cc, cz, czx, cx };

    auto& camera = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);
    sm::vec3 ray_dir = m_vp.TransPos3ScreenToDir(
        sm::vec2(static_cast<float>(x), static_cast<float>(y)), *camera
    );
    sm::Ray ray(camera->GetPos(), ray_dir);
    sm::mat4 mat = m_ori_wmat_no_scale;
    sm::vec3 cross;
    if (sm::ray_polygon_intersect_both_faces(mat, face_xy, 4, ray, &cross)) {
        pos = cross;
        sm::Plane face_plane(
            mat * face_xy[0],
            mat * face_xy[1],
            mat * face_xy[2]
        );
        sm::ray_plane_intersect_both_faces(ray, face_plane, &pos);
        return POINT_QUERY_XY;
    } else if (sm::ray_polygon_intersect_both_faces(mat, face_yz, 4, ray, &cross)) {
        pos = cross;
        sm::Plane face_plane(
            mat * face_yz[0],
            mat * face_yz[1],
            mat * face_yz[2]
        );
        sm::ray_plane_intersect_both_faces(ray, face_plane, &pos);
        return POINT_QUERY_YZ;
    } else if (sm::ray_polygon_intersect_both_faces(mat, face_zx, 4, ray, &cross)) {
        pos = cross;
        sm::Plane face_plane(
            mat * face_zx[0],
            mat * face_zx[1],
            mat * face_zx[2]
        );
        sm::ray_plane_intersect_both_faces(ray, face_plane, &pos);
        return POINT_QUERY_ZX;
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

	sm::mat4 wmat = m_cb.get_origin_wmat();

	sm::vec3 trans, rotate, scale;
	wmat.Decompose(trans, rotate, scale);

	m_ori_wmat_scale     = sm::mat4::Scaled(scale.x, scale.y, scale.z);
    m_ori_wmat_rotate    = sm::mat4::Rotated(rotate.x, rotate.y, rotate.z);
    m_ori_wmat_translate = sm::mat4::Translated(trans.x, trans.y, trans.z);
	m_ori_wmat_no_scale = m_ori_wmat_rotate * m_ori_wmat_translate;

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
}

void TranslateAxisState::Translate(int x, int y)
{
	if (m_camera->TypeID() == pt0::GetCamTypeID<pt3::PerspCam>())
	{
		auto p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);

        sm::vec3 ray_dir;
        switch (m_op_type)
        {
        case POINT_QUERY_X:
        case POINT_QUERY_Y:
        case POINT_QUERY_Z:
        {
            auto end = m_cam2d->TransPosScreenToProject(x, y,
                static_cast<int>(m_vp.Width()), static_cast<int>(m_vp.Height()));
            auto fixed_end = m_first_pos2 + m_first_dir2 * ((end - m_first_pos2).Dot(m_first_dir2));
            auto screen_fixed_end = m_cam2d->TransPosProjectToScreen(fixed_end,
                static_cast<int>(m_vp.Width()), static_cast<int>(m_vp.Height()));
            ray_dir = m_vp.TransPos3ScreenToDir(screen_fixed_end, *p_cam);
        }
            break;
        case POINT_QUERY_YZ:
        case POINT_QUERY_ZX:
        case POINT_QUERY_XY:
            ray_dir = m_vp.TransPos3ScreenToDir(
                sm::vec2(static_cast<float>(x), static_cast<float>(y)), *p_cam
            );
            break;
        }
		sm::Ray ray(p_cam->GetPos(), ray_dir);

        sm::Plane cross_face;
        switch (m_op_type)
        {
        case POINT_QUERY_X:
        case POINT_QUERY_Y:
        case POINT_QUERY_Z:
        {
            sm::vec3 cross_face_pos = m_move_path3d.origin + m_move_path3d.dir.Cross(ray_dir);
            cross_face.Build(m_move_path3d.origin, m_move_path3d.origin + m_move_path3d.dir, cross_face_pos);
        }
            break;
        case POINT_QUERY_YZ:
            cross_face.Build(m_ori_wmat_rotate * sm::vec3(1, 0, 0), m_ori_wmat_no_scale * sm::vec3(0, 0, 0));
            break;
        case POINT_QUERY_ZX:
            cross_face.Build(m_ori_wmat_rotate * sm::vec3(0, 1, 0), m_ori_wmat_no_scale * sm::vec3(0, 0, 0));
            break;
        case POINT_QUERY_XY:
            cross_face.Build(m_ori_wmat_rotate * sm::vec3(0, 0, 1), m_ori_wmat_no_scale * sm::vec3(0, 0, 0));
            break;
        }

		sm::vec3 offset;
		sm::vec3 cross;
		if (sm::ray_plane_intersect_both_faces(ray, cross_face, &cross)) {
			offset = cross - m_last_pos3;
			m_last_pos3 = cross;
		}

		// world pos to local
		m_cb.translate(m_ori_wmat_scale.Inverted() * offset);

	//	m_selection.Traverse([&](const ee0::GameObjWithPos& nwp)->bool
	//	{
	//#ifndef GAME_OBJ_ECS
	//		auto& ctrans = nwp.GetNode()->GetUniqueComp<n3::CompTransform>();
	//		ctrans.Translate(offset);
	//#endif // GAME_OBJ_ECS
	//		return true;
	//	});

		m_ori_wmat_no_scale = m_ori_wmat_no_scale * sm::mat4::Translated(offset.x, offset.y, offset.z);
	}
}

sm::vec2 TranslateAxisState::GetCtrlPos2D(const sm::mat4& cam_mat, AxisNodeType type) const
{
	return m_vp.TransPosProj3ToProj2(m_ori_wmat_no_scale * GetCtrlPos3D(type), cam_mat);
}

sm::vec3 TranslateAxisState::GetCtrlPos3D(AxisNodeType type) const
{
	sm::vec3 pos;
    const float len = CalcCoordAxisLen();
	switch (type)
	{
	case AXIS_CENTER:
		pos.Set(0, 0, 0);
		break;
	case AXIS_X:
		pos.Set(len, 0, 0);
		break;
	case AXIS_Y:
		pos.Set(0, len, 0);
		break;
	case AXIS_Z:
		pos.Set(0, 0, -len);
		break;
	}
	return pos;
}

float TranslateAxisState::CalcCoordAxisLen() const
{
    auto& cam_pos = std::static_pointer_cast<pt3::PerspCam>(m_camera)->GetPos();
    float dis = sm::dis_pos3_to_pos3(m_ori_wmat_no_scale * sm::vec3(0, 0, 0), cam_pos);
    return dis * m_cfg.arc_radius * 0.5f;
}

}