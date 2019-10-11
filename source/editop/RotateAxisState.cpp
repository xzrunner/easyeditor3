#include "ee3/RotateAxisState.h"

#include <ee0/SubjectMgr.h>
#include <ee0/MessageID.h>

#include <SM_Cube.h>
#include <SM_Calc.h>
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

namespace ee3
{

RotateAxisState::RotateAxisState(const std::shared_ptr<pt0::Camera>& camera,
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
	m_last_pos.MakeInvalid();

	m_cam2d->OnSize(m_vp.Width(), m_vp.Height());
}

bool RotateAxisState::OnMousePress(int x, int y)
{
	m_op_type = PointQuery(x, y);
	if (m_op_type != POINT_QUERY_NULL) {
		m_last_pos = m_cam2d->TransPosScreenToProject(x, y,
			static_cast<int>(m_vp.Width()), static_cast<int>(m_vp.Height()));
		return true;
	} else {
		return false;
	}

	return false;
}

bool RotateAxisState::OnMouseRelease(int x, int y)
{
	m_op_type = POINT_QUERY_NULL;

	return false;
}

bool RotateAxisState::OnMouseDrag(int x, int y)
{
	auto pos = m_cam2d->TransPosScreenToProject(x, y,
		static_cast<int>(m_vp.Width()), static_cast<int>(m_vp.Height()));
	Rotate(m_last_pos, pos);
	m_last_pos = pos;

	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

	return false;
}

bool RotateAxisState::OnActive(bool active)
{
	if (active) {
		UpdateSelectionSetInfo();
	}

	m_active = active;

	return false;
}

bool RotateAxisState::OnDraw() const
{
	if (!m_cb.is_need_draw()) {
		return false;
	}

	tess::Painter pt;

	auto cam_mat = m_camera->GetProjectionMat() * m_camera->GetViewMat();

	const float line_width = 2.0f;

    const float len = CalcCoordAxisLen();

	auto c = m_vp.TransPosProj3ToProj2(m_ori_wmat_no_scale * sm::vec3(0, 0, 0), cam_mat);
	auto x = m_vp.TransPosProj3ToProj2(m_ori_wmat_no_scale * sm::vec3(len, 0, 0), cam_mat);
	auto y = m_vp.TransPosProj3ToProj2(m_ori_wmat_no_scale * sm::vec3(0, len, 0), cam_mat);
	auto z = m_vp.TransPosProj3ToProj2(m_ori_wmat_no_scale * sm::vec3(0, 0, -len), cam_mat);

	// draw edges
	// axis
	pt.AddLine(c, x, 0xff0000ff, line_width);
	pt.AddLine(c, y, 0xff00ff00, line_width);
	pt.AddLine(c, z, 0xffff0000, line_width);
	// arc
	auto trans_func = [&](const sm::vec3& pos3)->sm::vec2 {
		return m_vp.TransPosProj3ToProj2(m_ori_wmat_no_scale * pos3, cam_mat);
	};
	pt.AddArc3D(sm::mat4::RotatedAxis(sm::vec3(0, 1, 0), SM_PI * 0.5f),
		len, 0, SM_PI * 0.5f, trans_func, 0xff0000ff, line_width);
	pt.AddArc3D(sm::mat4::RotatedAxis(sm::vec3(1, 0, 0), -SM_PI * 0.5f),
		len, 0, SM_PI * 0.5f, trans_func, 0xff00ff00, line_width);
	pt.AddArc3D(sm::mat4(),
		len, 0, SM_PI * 0.5f, trans_func, 0xffff0000, line_width);

	// draw nodes
	// x, green
	pt.AddCircleFilled(x, m_cfg.node_radius, 0xff00ff00);
	// y, blue
	pt.AddCircleFilled(y, m_cfg.node_radius, 0xffff0000);
	// z, red
	pt.AddCircleFilled(z, m_cfg.node_radius, 0xff0000ff);

	pt2::RenderSystem::DrawPainter(pt);

	return false;
}

RotateAxisState::PointQueryType RotateAxisState::PointQuery(int x, int y) const
{
	auto cam_mat = m_camera->GetProjectionMat() * m_camera->GetViewMat();

    const float len = CalcCoordAxisLen();

	auto proj2d = m_cam2d->TransPosScreenToProject(x, y,
		static_cast<int>(m_vp.Width()), static_cast<int>(m_vp.Height()));
	// x, green
	auto pos2d = m_vp.TransPosProj3ToProj2(m_ori_wmat_no_scale * sm::vec3(len, 0, 0), cam_mat);
	if (sm::dis_pos_to_pos(pos2d, proj2d) < m_cfg.node_radius) {
		return POINT_QUERY_Y;
	}
	// y, blue
	pos2d = m_vp.TransPosProj3ToProj2(m_ori_wmat_no_scale * sm::vec3(0, len, 0), cam_mat);
	if (sm::dis_pos_to_pos(pos2d, proj2d) < m_cfg.node_radius) {
		return POINT_QUERY_Z;
	}
	// z, red
	pos2d = m_vp.TransPosProj3ToProj2(m_ori_wmat_no_scale * sm::vec3(0, 0, -len), cam_mat);
	if (sm::dis_pos_to_pos(pos2d, proj2d) < m_cfg.node_radius) {
		return POINT_QUERY_X;
	}

	return POINT_QUERY_NULL;
}

void RotateAxisState::UpdateSelectionSetInfo()
{
	sm::mat4 wmat = m_cb.get_origin_wmat();

	sm::vec3 trans, rotate, scale;
	wmat.Decompose(trans, rotate, scale);

	auto rot_mat = sm::mat4::Rotated(rotate.x, rotate.y, rotate.z);
	auto trans_mat = sm::mat4::Translated(trans.x, trans.y, trans.z);
	m_ori_wmat_no_scale = trans_mat * rot_mat;

	auto cam_mat = m_camera->GetProjectionMat() * m_camera->GetViewMat();
	m_pos2d = m_vp.TransPosProj3ToProj2(trans, cam_mat);
}

void RotateAxisState::Rotate(const sm::vec2& start, const sm::vec2& end)
{
    const float len = CalcCoordAxisLen();
	auto cam_mat = m_camera->GetProjectionMat() * m_camera->GetViewMat();
	auto raidus = m_vp.TransPosProj3ToProj2(sm::vec3(len, 0, 0), cam_mat);
	float angle = atan(sm::dis_pos_to_pos(start, end) / raidus.Length());
	if ((start - m_pos2d).Cross(end - m_pos2d) < 0) {
		angle = -angle;
	}

	sm::Quaternion delta;
	switch (m_op_type)
	{
	case POINT_QUERY_X:
		delta = sm::Quaternion::CreateFromEulerAngle(0, angle, 0);
		break;
	case POINT_QUERY_Y:
		delta = sm::Quaternion::CreateFromEulerAngle(0, 0, angle);
		break;
	case POINT_QUERY_Z:
		delta = sm::Quaternion::CreateFromEulerAngle(-angle, 0, 0);
		break;
	}

//	m_selection.Traverse([&](const ee0::GameObjWithPos& nwp)->bool
//	{
//#ifndef GAME_OBJ_ECS
//		auto& ctrans = nwp.GetNode()->GetUniqueComp<n3::CompTransform>();
//		ctrans.Rotate(-delta);
//#endif // GAME_OBJ_ECS
//
//		return true;
//	});

	m_cb.rotate(-delta);
}

float RotateAxisState::CalcCoordAxisLen() const
{
    auto& cam_pos = std::static_pointer_cast<pt3::PerspCam>(m_camera)->GetPos();
    float dis = sm::dis_pos3_to_pos3(m_ori_wmat_no_scale * sm::vec3(0, 0, 0), cam_pos);
    return dis * m_cfg.arc_radius * 0.5f;
}

}