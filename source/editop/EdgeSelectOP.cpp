#include "ee3/EdgeSelectOP.h"

namespace ee3
{
namespace mesh
{

EdgeSelectOP::EdgeSelectOP(const pt0::CameraPtr& cam,
	                       const pt3::Viewport& vp, 
	                       const ee0::SubjectMgrPtr& sub_mgr, 
	                       const MeshPointQuery::Selected& selected)
	: MeshSelectBaseOP<BrushEdge>(cam, vp, sub_mgr, selected)
{
}

void EdgeSelectOP::DrawImpl(const quake::MapBrush& brush, const sm::mat4& cam_mat) const
{
	// all edges
	pt2::PrimitiveDraw::SetColor(UNSELECT_COLOR);
	for (auto& face : brush.faces) 
	{
		auto& vs = face->vertices;
		for (int i = 0, n = vs.size(); i < n; ++i) {
			BrushEdge(vs[i], vs[(i + 1) % n]).Project(m_vp, cam_mat,
				[&](const sm::vec2& b, const sm::vec2& e, const sm::vec2& mid) {
				pt2::PrimitiveDraw::Line(nullptr, b, e);
				pt2::PrimitiveDraw::Circle(nullptr, mid, NODE_DRAW_RADIUS, true);
			});
		}
	}
	// selecting
	if (m_selecting) 
	{
		m_selecting.Project(m_vp, cam_mat,
			[&](const sm::vec2& b, const sm::vec2& e, const sm::vec2& mid) {
			pt2::PrimitiveDraw::SetColor(SELECT_COLOR);
			pt2::PrimitiveDraw::Line(nullptr, b, e);
			pt2::PrimitiveDraw::Circle(nullptr, mid, NODE_QUERY_RADIUS, false);
		});
	}
	// selected
	pt2::PrimitiveDraw::SetColor(SELECT_COLOR);
	m_selected.Traverse([&](const BrushEdge& edge)->bool {
		edge.Project(m_vp, cam_mat,
			[&](const sm::vec2& b, const sm::vec2& e, const sm::vec2& mid) {
			pt2::PrimitiveDraw::SetColor(SELECT_COLOR);
			pt2::PrimitiveDraw::Line(nullptr, b, e);
			pt2::PrimitiveDraw::Circle(nullptr, mid, NODE_DRAW_RADIUS, true);
		});
		return true;
	});
}

BrushEdge EdgeSelectOP::QueryByPos(int x, int y) const
{
	auto brush = m_base_selected.GetBrush();
	if (!brush) {
		return BrushEdge();
	}

	auto pos = m_cam2d.TransPosScreenToProject(x, y,
		static_cast<int>(m_vp.Width()), static_cast<int>(m_vp.Height()));

	auto cam_mat = m_cam->GetModelViewMat() * m_cam->GetProjectionMat();
	for (auto& face : brush->faces)
	{
		auto& vs = face->vertices;
		for (int i = 0, n = vs.size(); i < n; ++i)
		{
			BrushEdge edge;
			BrushEdge(vs[i], vs[(i + 1) % n]).Project(m_vp, cam_mat,
				[&](const sm::vec2& b, const sm::vec2& e, const sm::vec2& mid) {
				if (sm::dis_pos_to_pos(mid, pos) < NODE_QUERY_RADIUS) {
					edge = BrushEdge(vs[i], vs[(i + 1) % n]);
				}
			});
			if (edge) {
				return edge;
			}
		}
	}

	return BrushEdge();
}

void EdgeSelectOP::QueryByRect(const sm::irect& rect, std::vector<BrushEdge>& selection) const
{
	auto brush = m_base_selected.GetBrush();
	if (!brush) {
		return;
	}

	auto r_min = m_cam2d.TransPosScreenToProject(rect.xmin, rect.ymin,
		static_cast<int>(m_vp.Width()), static_cast<int>(m_vp.Height()));
	auto r_max = m_cam2d.TransPosScreenToProject(rect.xmax, rect.ymax,
		static_cast<int>(m_vp.Width()), static_cast<int>(m_vp.Height()));
	sm::rect s_rect(r_min, r_max);

	auto cam_mat = m_cam->GetModelViewMat() * m_cam->GetProjectionMat();
	for (auto& face : brush->faces)
	{
		auto& vs = face->vertices;
		for (int i = 0, n = vs.size(); i < n; ++i)
		{
			BrushEdge(vs[i], vs[(i + 1) % n]).Project(m_vp, cam_mat,
				[&](const sm::vec2& b, const sm::vec2& e, const sm::vec2& mid) {
				if (sm::is_point_in_rect(mid, s_rect)) {
					selection.push_back(BrushEdge(vs[i], vs[(i + 1) % n]));
				}
			});
		}
	}
}

}
}