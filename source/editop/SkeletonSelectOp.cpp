#include "ee3/SkeletonSelectOp.h"

#include <ee0/SubjectMgr.h>
#include <ee0/MessageID.h>

#include <SM_Calc.h>
#include <model/SkeletalAnim.h>
#include <model/Model.h>
#include <model/ModelInstance.h>
#include <unirender2/RenderState.h>
#include <painting0/Camera.h>
#include <painting2/OrthoCamera.h>
#include <painting2/RenderSystem.h>
#include <painting3/Viewport.h>
#include <painting3/PerspCam.h>
#include <tessellation/Painter.h>

#include <array>

namespace
{

static const float NODE_DRAW_RADIUS  = 5;
static const float NODE_QUERY_RADIUS = 10;

}

namespace ee3
{

SkeletonSelectOp::SkeletonSelectOp(const ur2::Device& dev, ur2::Context& ctx,
                                   const std::shared_ptr<pt0::Camera>& camera,
	                               const pt3::Viewport& vp,
	                               const ee0::SubjectMgrPtr& sub_mgr)
	: ee0::EditOP(camera)
    , m_dev(dev)
    , m_ctx(ctx)
	, m_vp(vp)
	, m_sub_mgr(sub_mgr)
	, m_cam2d(std::make_shared<pt2::OrthoCamera>())
{
}

bool SkeletonSelectOp::OnDraw(const ur2::Device& dev, ur2::Context& ctx) const
{
	if (!m_model) {
		return false;
	}

	auto& model = m_model->GetModel();
	if (!model->ext) {
		return false;
	}

	auto cam_mat = m_camera->GetProjectionMat() * m_camera->GetViewMat();

	auto& bones = (static_cast<::model::SkeletalAnim*>(model->ext.get())->GetNodes());

	auto& g_trans = m_model->GetGlobalTrans();

	std::vector<sm::vec2> pos2;
	pos2.reserve(g_trans.size());
	for (auto& t : g_trans)
	{
		auto p3 = t * sm::vec3(0, 0, 0);
		auto p2 = m_vp.TransPosProj3ToProj2(p3, cam_mat);
		pos2.push_back(p2);
	}

	tess::Painter pt;
	for (int i = 0, n = bones.size(); i < n; ++i)
	{
		auto& p_pos2 = pos2[i];

		// point
		float radius = 0;
		uint32_t color = 0xffffffff;
		if (i == m_selecting || i == m_selected) {
			radius = NODE_DRAW_RADIUS * 2;
			color = 0xff00ffff;
		} else {
			radius = NODE_DRAW_RADIUS;
			color = 0xffffff00;
		}

		pt.AddCircleFilled(p_pos2, radius, color);

		// edge
		for (auto& child : bones[i]->children)
		{
			auto& c_pos2 = pos2[child];

			auto dir = (c_pos2 - p_pos2).Normalized();
			float len = std::min(sm::dis_pos_to_pos(c_pos2, p_pos2) * 0.05f, 10.0f);
			auto p_pos2_l = sm::rotate_vector_right_angle(dir, true) * len + p_pos2;
			auto p_pos2_r = sm::rotate_vector_right_angle(dir, false) * len + p_pos2;
			std::array<sm::vec2, 3> triangle = {
				p_pos2_l, p_pos2_r, c_pos2
			};

            pt.AddPolygonFilled(triangle.data(), triangle.size(), 0xffff00ff);
		}
	}

    ur2::RenderState rs;
	pt2::RenderSystem::DrawPainter(m_dev, m_ctx, rs, pt);

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

	auto& model = m_model->GetModel();
	if (!model->ext) {
		return -1;
	}

	auto pos = m_cam2d->TransPosScreenToProject(x, y,
		static_cast<int>(m_vp.Width()), static_cast<int>(m_vp.Height()));
	auto cam_mat = cam.GetProjectionMat() * cam.GetViewMat();

	auto& g_trans = m_model->GetGlobalTrans();
	auto& bones = (static_cast<::model::SkeletalAnim*>(model->ext.get())->GetNodes());
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