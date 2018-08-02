#include "ee3/VertexSelectOP.h"

#include <ee0/SubjectMgr.h>
#include <ee0/MessageID.h>
#include <ee2/DrawSelectRectState.h>

#include <painting2/PrimitiveDraw.h>
#include <painting3/ICamera.h>
#include <painting3/Viewport.h>
#include <SM_Calc.h>
#include <model/Model.h>
#include <model/MapLoader.h>

namespace
{

static const float NODE_RADIUS  = 3;
static const float QUERY_RADIUS = 6;

}

namespace ee3
{

VertexSelectOP::VertexSelectOP(pt3::ICamera& cam, const pt3::Viewport& vp,
	                           const ee0::SubjectMgrPtr& sub_mgr,
	                           const MeshPointQuery::Selected& selected)
	: m_cam(cam)
	, m_vp(vp)
	, m_sub_mgr(sub_mgr)
	, m_base_selected(selected)
{
	m_cam2d.OnSize(static_cast<int>(m_vp.Width()), static_cast<int>(m_vp.Height()));

	m_draw_state = std::make_unique<ee2::DrawSelectRectState>(m_cam2d, m_sub_mgr);
}

bool VertexSelectOP::OnMouseLeftDown(int x, int y)
{
	if (ee0::EditOP::OnMouseLeftDown(x, y)) {
		return true;
	}

	m_draw_state->OnMousePress(x, y);

	// todo
	if (!m_base_selected.poly) {
		return false;
	}

	m_first_pos.Set(x, y);

	m_selected.Clear();
	auto selected = QueryByPos(x, y);
	if (selected) {
		m_selected.Add(selected);
	}

	return false;
}

bool VertexSelectOP::OnMouseLeftUp(int x, int y)
{
	if (ee0::EditOP::OnMouseLeftUp(x, y)) {
		return true;
	}

	m_draw_state->OnMouseRelease(x, y);

	// rect select
	sm::ivec2 end(x, y);
	if (end != m_first_pos)
	{
		m_selected.Clear();
		std::vector<quake::BrushVertexPtr> selection;
		sm::irect r(m_first_pos, end);
		QueryByRect(r, selection);
		for (auto& v : selection) {
			m_selected.Add(v);
		}
	}

	return false;
}

bool VertexSelectOP::OnMouseMove(int x, int y)
{
	if (ee0::EditOP::OnMouseMove(x, y)) {
		return true;
	}

	// todo
	if (!m_base_selected.poly) {
		return false;
	}

	auto selecting = QueryByPos(x, y);
	if (m_selecting != selecting) {
		m_selecting = QueryByPos(x, y);
		m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
	}

	return false;
}

bool VertexSelectOP::OnMouseDrag(int x, int y)
{
	if (ee0::EditOP::OnMouseDrag(x, y)) {
		return true;
	}

	m_draw_state->OnMouseDrag(x, y);

	return false;
}

bool VertexSelectOP::OnDraw() const
{
	if (ee0::EditOP::OnDraw()) {
		return true;
	}

	if (m_base_selected.brush_idx < 0) {
		m_draw_state->OnDraw();
		return false;
	}

	auto cam_mat = m_cam.GetModelViewMat() * m_cam.GetProjectionMat();

	auto brush = m_base_selected.GetBrush();
	assert(brush);

	// nodes border
	pt2::PrimitiveDraw::SetColor(0xff00ffff);
	for (auto& v : brush->vertices) {
		auto pos = m_vp.TransPosProj3ToProj2(v->pos * model::MapLoader::VERTEX_SCALE, cam_mat);
		pt2::PrimitiveDraw::Circle(nullptr, pos, NODE_RADIUS, true);
	}
	// selecting
	if (m_selecting) {
		pt2::PrimitiveDraw::SetColor(0xff0000ff);
		auto pos = m_vp.TransPosProj3ToProj2(m_selecting->pos * model::MapLoader::VERTEX_SCALE, cam_mat);
		pt2::PrimitiveDraw::Circle(nullptr, pos, QUERY_RADIUS, false);
	}
	// selected
	pt2::PrimitiveDraw::SetColor(0xff0000ff);
	m_selected.Traverse([&](const quake::BrushVertexPtr& vert)->bool {
		auto pos = m_vp.TransPosProj3ToProj2(vert->pos * model::MapLoader::VERTEX_SCALE, cam_mat);
		pt2::PrimitiveDraw::Circle(nullptr, pos, NODE_RADIUS, true);
		return true;
	});

	m_draw_state->OnDraw();

	return false;
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
		if (sm::dis_pos_to_pos(p, pos) < QUERY_RADIUS) {
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