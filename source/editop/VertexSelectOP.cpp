#include "ee3/VertexSelectOP.h"

namespace ee3
{
namespace mesh
{

VertexSelectOP::VertexSelectOP(pt3::ICamera& cam, const pt3::Viewport& vp,
	                           const ee0::SubjectMgrPtr& sub_mgr,
	                           const MeshPointQuery::Selected& selected)
	: MeshSelectBaseOP<quake::BrushVertexPtr>(cam, vp, sub_mgr, selected)
{
	m_selecting = nullptr;
}

void VertexSelectOP::DrawImpl(const quake::MapBrush& brush, const sm::mat4& cam_mat) const
{
	// all nodes
	pt2::PrimitiveDraw::SetColor(UNSELECT_COLOR);
	for (auto& v : brush.vertices) {
		auto pos = m_vp.TransPosProj3ToProj2(v->pos * model::MapLoader::VERTEX_SCALE, cam_mat);
		pt2::PrimitiveDraw::Circle(nullptr, pos, NODE_DRAW_RADIUS, true);
	}
	// selecting
	if (m_selecting) 
	{
		pt2::PrimitiveDraw::SetColor(SELECT_COLOR);
		auto pos = m_vp.TransPosProj3ToProj2(m_selecting->pos * model::MapLoader::VERTEX_SCALE, cam_mat);
		pt2::PrimitiveDraw::Circle(nullptr, pos, NODE_QUERY_RADIUS, false);
	}
	// selected
	pt2::PrimitiveDraw::SetColor(SELECT_COLOR);
	m_selected.Traverse([&](const quake::BrushVertexPtr& vert)->bool {
		auto pos = m_vp.TransPosProj3ToProj2(vert->pos * model::MapLoader::VERTEX_SCALE, cam_mat);
		pt2::PrimitiveDraw::Circle(nullptr, pos, NODE_DRAW_RADIUS, true);
		return true;
	});
}

quake::BrushVertexPtr VertexSelectOP::QueryByPos(int x, int y) const
{
	auto pos = m_cam2d.TransPosScreenToProject(x, y,
		static_cast<int>(m_vp.Width()), static_cast<int>(m_vp.Height()));

	auto cam_mat = m_cam.GetModelViewMat() * m_cam.GetProjectionMat();

	auto brush = m_base_selected.GetBrush();
	assert(brush);
	for (auto& v : brush->vertices) {
		auto p = m_vp.TransPosProj3ToProj2(v->pos * model::MapLoader::VERTEX_SCALE, cam_mat);
		if (sm::dis_pos_to_pos(p, pos) < NODE_QUERY_RADIUS) {
			return v;
		}
	}

	return nullptr;
}

void VertexSelectOP::QueryByRect(const sm::irect& rect, std::vector<quake::BrushVertexPtr>& selection) const
{
	auto r_min = m_cam2d.TransPosScreenToProject(rect.xmin, rect.ymin,
		static_cast<int>(m_vp.Width()), static_cast<int>(m_vp.Height()));
	auto r_max = m_cam2d.TransPosScreenToProject(rect.xmax, rect.ymax,
		static_cast<int>(m_vp.Width()), static_cast<int>(m_vp.Height()));
	sm::rect s_rect(r_min, r_max);

	auto cam_mat = m_cam.GetModelViewMat() * m_cam.GetProjectionMat();

	auto brush = m_base_selected.GetBrush();
	assert(brush);
	for (auto& v : brush->vertices) {
		auto p = m_vp.TransPosProj3ToProj2(v->pos * model::MapLoader::VERTEX_SCALE, cam_mat);
		if (sm::is_point_in_rect(p, s_rect)) {
			selection.push_back(v);
		}
	}
}

}
}