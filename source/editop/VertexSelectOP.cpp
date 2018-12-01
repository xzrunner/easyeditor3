#include "ee3/VertexSelectOP.h"

#include <tessellation/Painter.h>
#include <painting2/OrthoCamera.h>
#include <painting2/RenderSystem.h>

namespace ee3
{
namespace mesh
{

VertexSelectOP::VertexSelectOP(const std::shared_ptr<pt0::Camera>& camera, const pt3::Viewport& vp,
	                           const ee0::SubjectMgrPtr& sub_mgr,
	                           const MeshPointQuery::Selected& selected)
	: MeshSelectBaseOP<quake::BrushVertexPtr>(camera, vp, sub_mgr, selected)
{
	m_selecting = nullptr;
}

void VertexSelectOP::DrawImpl(const quake::MapBrush& brush, const sm::mat4& cam_mat) const
{
	tess::Painter pt;

	// all nodes
	for (auto& v : brush.vertices) {
		auto pos = m_vp.TransPosProj3ToProj2(v->pos * model::MapBuilder::VERTEX_SCALE, cam_mat);
		pt.AddCircleFilled(pos, NODE_DRAW_RADIUS, UNSELECT_COLOR);
	}
	// selecting
	if (m_selecting)
	{
		auto pos = m_vp.TransPosProj3ToProj2(m_selecting->pos * model::MapBuilder::VERTEX_SCALE, cam_mat);
		pt.AddCircle(pos, NODE_QUERY_RADIUS, SELECT_COLOR);
	}
	// selected
	m_selected.Traverse([&](const quake::BrushVertexPtr& vert)->bool {
		auto pos = m_vp.TransPosProj3ToProj2(vert->pos * model::MapBuilder::VERTEX_SCALE, cam_mat);
		pt.AddCircleFilled(pos, NODE_DRAW_RADIUS, SELECT_COLOR);
		return true;
	});


	pt2::RenderSystem::DrawPainter(pt);
}

quake::BrushVertexPtr VertexSelectOP::QueryByPos(int x, int y) const
{
	auto brush = m_base_selected.GetBrush();
	if (!brush) {
		return nullptr;
	}

	auto pos = m_cam2d->TransPosScreenToProject(x, y,
		static_cast<int>(m_vp.Width()), static_cast<int>(m_vp.Height()));

	auto cam_mat = m_camera->GetViewMat() * m_camera->GetProjectionMat();

	for (auto& v : brush->vertices) {
		auto p = m_vp.TransPosProj3ToProj2(v->pos * model::MapBuilder::VERTEX_SCALE, cam_mat);
		if (sm::dis_pos_to_pos(p, pos) < NODE_QUERY_RADIUS) {
			return v;
		}
	}

	return nullptr;
}

void VertexSelectOP::QueryByRect(const sm::irect& rect, std::vector<quake::BrushVertexPtr>& selection) const
{
	auto brush = m_base_selected.GetBrush();
	if (!brush) {
		return;
	}

	auto r_min = m_cam2d->TransPosScreenToProject(rect.xmin, rect.ymin,
		static_cast<int>(m_vp.Width()), static_cast<int>(m_vp.Height()));
	auto r_max = m_cam2d->TransPosScreenToProject(rect.xmax, rect.ymax,
		static_cast<int>(m_vp.Width()), static_cast<int>(m_vp.Height()));
	sm::rect s_rect(r_min, r_max);

	auto cam_mat = m_camera->GetViewMat() * m_camera->GetProjectionMat();
	for (auto& v : brush->vertices) {
		auto p = m_vp.TransPosProj3ToProj2(v->pos * model::MapBuilder::VERTEX_SCALE, cam_mat);
		if (sm::is_point_in_rect(p, s_rect)) {
			selection.push_back(v);
		}
	}
}

}
}