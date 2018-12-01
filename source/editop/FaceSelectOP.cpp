#include "ee3/FaceSelectOP.h"

#include <tessellation/Painter.h>
#include <painting2/OrthoCamera.h>
#include <painting2/RenderSystem.h>

namespace ee3
{
namespace mesh
{

FaceSelectOP::FaceSelectOP(const std::shared_ptr<pt0::Camera>& camera, const pt3::Viewport& vp,
	                       const ee0::SubjectMgrPtr& sub_mgr,
	                       const MeshPointQuery::Selected& selected)
	: MeshSelectBaseOP<quake::BrushFacePtr>(camera, vp, sub_mgr, selected)
{
	m_selecting = nullptr;
}

void FaceSelectOP::DrawImpl(const quake::MapBrush& brush, const sm::mat4& cam_mat) const
{
	tess::Painter pt;
	// all nodes
	for (auto& f : brush.faces)
	{
		auto center = CalcFaceCenter(*f, cam_mat);
		DrawFace(pt, *f, LIGHT_UNSELECT_COLOR, cam_mat);
		pt.AddCircleFilled(center, NODE_DRAW_RADIUS, UNSELECT_COLOR);
	}
	// selecting
	if (m_selecting)
	{
		auto center = CalcFaceCenter(*m_selecting, cam_mat);
		pt.AddCircle(center, NODE_QUERY_RADIUS, SELECT_COLOR);
	}
	// selected
	m_selected.Traverse([&](const quake::BrushFacePtr& face)->bool
	{
		DrawFace(pt, *face, LIGHT_SELECT_COLOR, cam_mat);
		auto center = CalcFaceCenter(*face, cam_mat);
		pt.AddCircleFilled(center, NODE_DRAW_RADIUS, SELECT_COLOR);
		return true;
	});
	pt2::RenderSystem::DrawPainter(pt);
}

quake::BrushFacePtr FaceSelectOP::QueryByPos(int x, int y) const
{
	auto brush = m_base_selected.GetBrush();
	if (!brush) {
		return nullptr;
	}

	auto pos = m_cam2d->TransPosScreenToProject(x, y,
		static_cast<int>(m_vp.Width()), static_cast<int>(m_vp.Height()));

	auto cam_mat = m_camera->GetViewMat() * m_camera->GetProjectionMat();
	for (auto& f : brush->faces) {
		auto center = CalcFaceCenter(*f, cam_mat);
		if (sm::dis_pos_to_pos(center, pos) < NODE_QUERY_RADIUS) {
			return f;
		}
	}

	return nullptr;
}

void FaceSelectOP::QueryByRect(const sm::irect& rect, std::vector<quake::BrushFacePtr>& selection) const
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
	for (auto& f : brush->faces) {
		auto center = CalcFaceCenter(*f, cam_mat);
		if (sm::is_point_in_rect(center, s_rect)) {
			selection.push_back(f);
		}
	}
}

sm::vec2 FaceSelectOP::CalcFaceCenter(const quake::BrushFace& face, const sm::mat4& cam_mat) const
{
	if (face.vertices.empty()) {
		return sm::vec2(0, 0);
	}

	sm::vec3 center;
	for (auto& v : face.vertices) {
		center += v->pos;
	}
	center /= face.vertices.size();

	return m_vp.TransPosProj3ToProj2(center * model::MapBuilder::VERTEX_SCALE, cam_mat);
}

void FaceSelectOP::DrawFace(tess::Painter& pt, const quake::BrushFace& face, uint32_t color, const sm::mat4& cam_mat) const
{
	std::vector<sm::vec2> polygon;
	polygon.reserve(face.vertices.size());
	for (auto& v : face.vertices) {
		auto p3 = v->pos * model::MapBuilder::VERTEX_SCALE;
		polygon.push_back(m_vp.TransPosProj3ToProj2(p3, cam_mat));
	}
	pt.AddPolygonFilled(polygon.data(), polygon.size(), color);
}

}
}