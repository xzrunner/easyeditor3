#include "ee3/SkeletonSelectOp.h"

#include <ee0/SubjectMgr.h>
#include <ee0/MessageID.h>

#include <SM_Calc.h>
#include <model/SkeletalAnim.h>
#include <model/Model.h>
#include <model/ModelInstance.h>
#include <painting0/Camera.h>
#include <painting2/OrthoCamera.h>
#include <painting2/PrimitiveDraw.h>
#include <painting3/Viewport.h>
#include <painting3/PrimitiveDraw.h>

namespace
{

static const float NODE_DRAW_RADIUS  = 10;
static const float NODE_QUERY_RADIUS = 20;

}

namespace ee3
{

SkeletonSelectOp::SkeletonSelectOp(const std::shared_ptr<pt0::Camera>& camera,
	                               const pt3::Viewport& vp,
	                               const ee0::SubjectMgrPtr& sub_mgr)
	: ee0::EditOP(camera)
	, m_vp(vp)
	, m_sub_mgr(sub_mgr)
	, m_cam2d(std::make_shared<pt2::OrthoCamera>())
{
}

bool SkeletonSelectOp::OnDraw() const
{
	if (!m_model) {
		return false;
	}

	auto cam_mat = m_camera->GetModelViewMat() * m_camera->GetProjectionMat();

	auto& bones = (static_cast<::model::SkeletalAnim*>(m_model->GetModel()->ext.get())->GetAllNodes());

	auto& g_trans = m_model->GetGlobalTrans();

	std::vector<sm::vec2> pos2;
	pos2.reserve(g_trans.size());
	for (auto& t : g_trans)
	{
		auto p3 = t * sm::vec3(0, 0, 0);
		auto p2 = m_vp.TransPosProj3ToProj2(p3, cam_mat);
		pos2.push_back(p2);
	}

	for (int i = 0, n = bones.size(); i < n; ++i)
	{
		auto& p_pos2 = pos2[i];

		// point
		if (i == m_selecting || i == m_selected) {
			// FIXME: set color will no use here, if not flush shader by call PointSize()
			pt2::PrimitiveDraw::PointSize(NODE_DRAW_RADIUS * 2);
			pt2::PrimitiveDraw::SetColor(0xff00ffff);
		} else {
			pt2::PrimitiveDraw::PointSize(NODE_DRAW_RADIUS);
			pt2::PrimitiveDraw::SetColor(0xffffff00);
		}
		pt2::PrimitiveDraw::Point(nullptr, p_pos2);

		// edge
		pt2::PrimitiveDraw::LineWidth(2);
		for (auto& child : bones[i]->children)
		{
			auto& c_pos2 = pos2[child];

			auto dir = (c_pos2 - p_pos2).Normalized();
			float len = sm::dis_pos_to_pos(c_pos2, p_pos2) * 0.05f;
			auto p_pos2_l = sm::rotate_vector_right_angle(dir, true) * len + p_pos2;
			auto p_pos2_r = sm::rotate_vector_right_angle(dir, false) * len + p_pos2;
			std::vector<sm::vec2> triangle = {
				p_pos2_l, p_pos2_r, c_pos2
			};

			pt2::PrimitiveDraw::SetColor(0xffff00ff);
			pt2::PrimitiveDraw::Polyline(nullptr, triangle, true);
		}
	}

	return false;
}

void SkeletonSelectOp::SetSelected(int selected)
{
	m_selected = selected;
	m_selecting = m_selected;

	OnActive();

	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
}

int SkeletonSelectOp::QueryJointByPos(const pt0::Camera& cam, int x, int y) const
{
	if (!m_model) {
		return -1;
	}

	auto pos = m_cam2d->TransPosScreenToProject(x, y,
		static_cast<int>(m_vp.Width()), static_cast<int>(m_vp.Height()));
	auto cam_mat = cam.GetModelViewMat() * cam.GetProjectionMat();

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