#include "ee3/NodeRotateOP.h"
#include "ee3/CamZoomState.h"

#include <ee0/WxStagePage.h>
#include <ee0/MessageID.h>
#include <ee0/SubjectMgr.h>

#include <SM_Cube.h>
#include <SM_Calc.h>
#include <unirender/Blackboard.h>
#include <unirender/RenderContext.h>
#include <shaderlab/Blackboard.h>
#include <shaderlab/RenderContext.h>
#include <shaderlab/ShaderMgr.h>
#include <shaderlab/Shader.h>
#include <node0/SceneNode.h>
#include <node3/CompAABB.h>
#include <node3/CompTransform.h>
#include <painting2/PrimitiveDraw.h>
#include <painting3/PrimitiveDraw.h>
#include <painting3/Camera.h>
#include <painting3/Viewport.h>

namespace
{

static const float ARC_RADIUS = 5;
static const float NODE_RADIUS = 5;

}

namespace ee3
{

NodeRotateOP::NodeRotateOP(ee0::WxStagePage& stage, pt3::Camera& cam,
	                       const pt3::Viewport& vp)
	: m_selection(stage.GetSelection())
	, m_sub_mgr(stage.GetSubjectMgr())
	, m_cam(cam)
	, m_vp(vp)
	, m_op_type(POINT_QUERY_NULL)
{
	m_cam_zoom_state = std::make_shared<ee3::CamZoomState>(
		cam, vp, m_sub_mgr);

	m_cam2d.OnSize(m_vp.Width(), m_vp.Height());
}

bool NodeRotateOP::OnMouseLeftDown(int x, int y)
{
	if (EditOP::OnMouseLeftDown(x, y)) {
		return true;
	}

	m_op_type = PointQuery(x, y);
	if (m_op_type != POINT_QUERY_NULL) {
		m_last_pos = m_cam2d.TransPosScreenToProject(x, y, m_vp.Width(), m_vp.Height());
	}

	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

	return false;
}

bool NodeRotateOP::OnMouseLeftUp(int x, int y)
{
	if (EditOP::OnMouseLeftUp(x, y)) {
		return true;
	}

	m_op_type = POINT_QUERY_NULL;

	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

	return false;
}

bool NodeRotateOP::OnMouseDrag(int x, int y)
{
	if (EditOP::OnMouseDrag(x, y)) {
		return true;
	}

	if (m_op_type != POINT_QUERY_NULL)
	{
		auto pos = m_cam2d.TransPosScreenToProject(x, y, m_vp.Width(), m_vp.Height());
		Rotate(m_last_pos, pos);
		m_last_pos = pos;

		m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
	}

	return false;
}

bool NodeRotateOP::OnMouseWheelRotation(int x, int y, int direction)
{
	if (EditOP::OnMouseWheelRotation(x, y, direction)) {
		return true;
	}

	return m_cam_zoom_state->OnMouseWheelRotation(x, y, direction);
}

bool NodeRotateOP::OnActive()
{
	if (EditOP::OnActive()) {
		return true;
	}

	InitSelectionCenter();

	return true;
}

bool NodeRotateOP::OnDraw() const
{
	if (EditOP::OnDraw()) {
		return true;
	}

	if (m_selection.IsEmpty()) {
		return false;
	}

	// 3d
	DrawEdges();

	// 2d
	auto& ur_rc = ur::Blackboard::Instance()->GetRenderContext();
	ur_rc.SetCull(ur::CULL_DISABLE);
	DrawNodes();
	sl::Blackboard::Instance()->GetRenderContext().GetShaderMgr().FlushShader();

	return true;
}

NodeRotateOP::PointQueryType NodeRotateOP::PointQuery(int x, int y) const
{
	auto cam_mat = m_cam.GetModelViewMat() * m_cam.GetProjectionMat();

	auto proj2d = m_cam2d.TransPosScreenToProject(x, y, m_vp.Width(), m_vp.Height());
	// x, green
	auto pos2d = m_vp.TransPosProj3ToProj2(sm::vec3(m_center.x + ARC_RADIUS, m_center.y, m_center.z), cam_mat);
	if (sm::dis_pos_to_pos(pos2d, proj2d) < NODE_RADIUS) {
		return POINT_QUERY_Y;
	}
	// y, blue
	pos2d = m_vp.TransPosProj3ToProj2(sm::vec3(m_center.x, m_center.y + ARC_RADIUS, m_center.z), cam_mat);
	if (sm::dis_pos_to_pos(pos2d, proj2d) < NODE_RADIUS) {
		return POINT_QUERY_Z;
	}
	// z, red
	pos2d = m_vp.TransPosProj3ToProj2(sm::vec3(m_center.x, m_center.y, m_center.z - ARC_RADIUS), cam_mat);
	if (sm::dis_pos_to_pos(pos2d, proj2d) < NODE_RADIUS) {
		return POINT_QUERY_X;
	}

	return POINT_QUERY_NULL;
}

void NodeRotateOP::InitSelectionCenter()
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

	int count = 0;
	m_selection.Traverse([&](const ee0::GameObjWithPos& opw)->bool
	{
		++count;
		auto aabb = opw.GetNode()->GetUniqueComp<n3::CompAABB>().GetAABB();
		auto pos = opw.GetNode()->GetUniqueComp<n3::CompTransform>().GetTransformMat() * aabb.Cube().Center();
		m_center += pos;
		return false;
	});
	m_center /= count;

	auto cam_mat = m_cam.GetModelViewMat() * m_cam.GetProjectionMat();
	m_center2d = m_vp.TransPosProj3ToProj2(m_center, cam_mat);
}

void NodeRotateOP::Rotate(const sm::vec2& start, const sm::vec2& end)
{
	auto cam_mat = m_cam.GetModelViewMat() * m_cam.GetProjectionMat();
	auto raidus = m_vp.TransPosProj3ToProj2(sm::vec3(ARC_RADIUS, 0, 0), cam_mat);
	float angle = atan(sm::dis_pos_to_pos(start, end) / raidus.Length());
	if ((start - m_center2d).Cross(end - m_center2d) < 0) {
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

	m_selection.Traverse([&](const ee0::GameObjWithPos& nwp)->bool
	{
#ifndef GAME_OBJ_ECS
		auto& ctrans = nwp.GetNode()->GetUniqueComp<n3::CompTransform>();
		ctrans.Rotate(-delta);
#endif // GAME_OBJ_ECS

		return true;
	});
}

void NodeRotateOP::DrawEdges() const
{
	// axis
	pt3::PrimitiveDraw::SetColor(0xff0000ff);
	pt3::PrimitiveDraw::Line(m_center, sm::vec3(m_center.x + ARC_RADIUS, m_center.y, m_center.z));
	pt3::PrimitiveDraw::SetColor(0xff00ff00);
	pt3::PrimitiveDraw::Line(m_center, sm::vec3(m_center.x, m_center.y + ARC_RADIUS, m_center.z));
	pt3::PrimitiveDraw::SetColor(0xffff0000);
	pt3::PrimitiveDraw::Line(m_center, sm::vec3(m_center.x, m_center.y, m_center.z - ARC_RADIUS));

	// arc
	pt3::PrimitiveDraw::SetColor(0xff0000ff);
	pt3::PrimitiveDraw::Arc(m_center, ARC_RADIUS, sm::vec3(0, 0, -1), sm::vec3(1, 0, 0), 0, SM_PI * 0.5f);
	pt3::PrimitiveDraw::SetColor(0xff00ff00);
	pt3::PrimitiveDraw::Arc(m_center, ARC_RADIUS, sm::vec3(1, 0, 0), sm::vec3(0, 1, 0), 0, SM_PI * 0.5f);
	pt3::PrimitiveDraw::SetColor(0xffff0000);
	pt3::PrimitiveDraw::Arc(m_center, ARC_RADIUS, sm::vec3(1, 0, 0), sm::vec3(0, 0, 1), 0, SM_PI * 0.5f);

}

void NodeRotateOP::DrawNodes() const
{
	auto cam_mat = m_cam.GetModelViewMat() * m_cam.GetProjectionMat();
	// x, green
	auto pos2d = m_vp.TransPosProj3ToProj2(sm::vec3(m_center.x + ARC_RADIUS, m_center.y, m_center.z), cam_mat);
	pt2::PrimitiveDraw::SetColor(0xff00ff00);
	pt2::PrimitiveDraw::Circle(nullptr, pos2d, NODE_RADIUS, true);
	// y, blue
	pos2d = m_vp.TransPosProj3ToProj2(sm::vec3(m_center.x, m_center.y + ARC_RADIUS, m_center.z), cam_mat);
	pt2::PrimitiveDraw::SetColor(0xffff0000);
	pt2::PrimitiveDraw::Circle(nullptr, pos2d, NODE_RADIUS, true);
	// z, red
	pos2d = m_vp.TransPosProj3ToProj2(sm::vec3(m_center.x, m_center.y, m_center.z - ARC_RADIUS), cam_mat);
	pt2::PrimitiveDraw::SetColor(0xff0000ff);
	pt2::PrimitiveDraw::Circle(nullptr, pos2d, NODE_RADIUS, true);
}

}