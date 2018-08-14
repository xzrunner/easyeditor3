#include "ee3/SkeletonOpImpl.h"

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

SkeletonOpImpl::SkeletonOpImpl(const pt3::Viewport& vp,
	                           const ee0::SubjectMgrPtr& sub_mgr)
	: m_vp(vp)
	, m_sub_mgr(sub_mgr)
	, m_cam2d(std::make_shared<pt2::OrthoCamera>())
{
}

int SkeletonOpImpl::QueryJointByPos(const pt0::Camera& cam, int x, int y) const
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

void SkeletonOpImpl::OnDraw() const
{
	if (!m_model) {
		return;
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
		pt2::PrimitiveDraw::LineWidth(3);
		for (auto& child : bones[i]->children)
		{
			auto c_pos = g_trans[child] * sm::vec3(0, 0, 0);
			pt3::PrimitiveDraw::SetColor(0xffff00ff);
			pt3::PrimitiveDraw::Line(p_pos, c_pos);
		}
	}
}

}