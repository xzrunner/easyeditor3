#include "ee3/MeshVertexOP.h"

#include <ee0/SubjectMgr.h>
#include <ee0/MessageID.h>

#include <painting2/PrimitiveDraw.h>
#include <painting3/Camera.h>
#include <painting3/Viewport.h>
#include <model/QuakeMapEntity.h>
#include <model/Model.h>
#include <model/MapLoader.h>
#include <SM_Calc.h>

namespace
{

static const float NODE_RADIUS  = 3;
static const float QUERY_RADIUS = 6;

}

namespace ee3
{

MeshVertexOP::MeshVertexOP(pt3::Camera& cam,
	                       const pt3::Viewport& vp,
	                       const ee0::SubjectMgrPtr& sub_mgr,
	                       const MeshPointQuery::Selected& selected)
	: m_cam(cam)
	, m_vp(vp)
	, m_sub_mgr(sub_mgr)
	, m_selected(selected)
{
	m_cam2d.OnSize(static_cast<int>(m_vp.Width()), static_cast<int>(m_vp.Height()));
}

bool MeshVertexOP::OnKeyDown(int key_code)
{
	if (ee0::EditOP::OnKeyDown(key_code)) {
		return true;
	}

	return false;
}

bool MeshVertexOP::OnKeyUp(int key_code)
{
	if (ee0::EditOP::OnKeyUp(key_code)) {
		return true;
	}

	return false;
}

bool MeshVertexOP::OnMouseLeftDown(int x, int y)
{
	if (ee0::EditOP::OnMouseLeftDown(x, y)) {
		return true;
	}

	// todo
	if (!m_selected.poly) {
		return false;
	}

	auto selected = QueryByPos(x, y);
	if (m_selected_vert != selected) {
		m_selected_vert = QueryByPos(x, y);
		m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
	}

	return false;
}

bool MeshVertexOP::OnMouseLeftUp(int x, int y)
{
	if (ee0::EditOP::OnMouseLeftUp(x, y)) {
		return true;
	}

	return false;
}

bool MeshVertexOP::OnMouseMove(int x, int y)
{
	if (ee0::EditOP::OnMouseMove(x, y)) {
		return true;
	}

	// todo
	if (!m_selected.poly) {
		return false;
	}

	auto selecting = QueryByPos(x, y);
	if (m_selecting_vert != selecting) {
		m_selecting_vert = QueryByPos(x, y);
		m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
	}

	return false;
}

bool MeshVertexOP::OnMouseDrag(int x, int y)
{
	if (ee0::EditOP::OnMouseDrag(x, y)) {
		return true;
	}

	return false;
}

bool MeshVertexOP::OnDraw() const
{
	if (ee0::EditOP::OnDraw()) {
		return true;
	}

	if (m_selected.brush_idx < 0) {
		return false;
	}

	auto cam_mat = m_cam.GetModelViewMat() * m_cam.GetProjectionMat();

	auto brush = m_selected.GetBrush();
	assert(brush);

	// nodes border
	pt2::PrimitiveDraw::SetColor(0xff00ffff);
	for (auto& v : brush->vertices) {
		auto pos = m_vp.TransPosProj3ToProj2(v->pos * model::MapLoader::VERTEX_SCALE, cam_mat);
		pt2::PrimitiveDraw::Circle(nullptr, pos, NODE_RADIUS, true);
	}
	// selecting
	if (m_selecting_vert) {
		pt2::PrimitiveDraw::SetColor(0xff0000ff);
		auto pos = m_vp.TransPosProj3ToProj2(m_selecting_vert->pos * model::MapLoader::VERTEX_SCALE, cam_mat);
		pt2::PrimitiveDraw::Circle(nullptr, pos, QUERY_RADIUS, false);
	}
	// selected
	if (m_selected_vert) {
		pt2::PrimitiveDraw::SetColor(0xff0000ff);
		auto pos = m_vp.TransPosProj3ToProj2(m_selected_vert->pos * model::MapLoader::VERTEX_SCALE, cam_mat);
		pt2::PrimitiveDraw::Circle(nullptr, pos, NODE_RADIUS, true);
	}

	return false;
}

quake::BrushVertexPtr MeshVertexOP::QueryByPos(int x, int y) const
{
	auto pos = m_cam2d.TransPosScreenToProject(x, y,
		static_cast<int>(m_vp.Width()), static_cast<int>(m_vp.Height()));

	auto cam_mat = m_cam.GetModelViewMat() * m_cam.GetProjectionMat();

	auto brush = m_selected.GetBrush();
	assert(brush);
	for (auto& v : brush->vertices) {
		auto p = m_vp.TransPosProj3ToProj2(v->pos * model::MapLoader::VERTEX_SCALE, cam_mat);
		if (sm::dis_pos_to_pos(p, pos) < QUERY_RADIUS) {
			return v;
		}
	}

	return nullptr;
}

}