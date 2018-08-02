#include "ee3/VertexTranslateOP.h"
#include "ee3/MeshEditStyle.h"

#include <ee0/MessageID.h>
#include <ee0/SubjectMgr.h>

#include <painting3/PerspCam.h>
#include <painting3/Viewport.h>
#include <model/MapLoader.h>
#include <SM_Calc.h>
#include <SM_Ray.h>
#include <SM_RayIntersect.h>

namespace ee3
{
namespace mesh
{

VertexTranslateOP::VertexTranslateOP(pt3::PerspCam& cam,
	                                 const pt3::Viewport& vp,
	                                 const ee0::SubjectMgrPtr& sub_mgr,
	                                 const MeshPointQuery::Selected& selected,
	                                 const ee0::SelectionSet<quake::BrushVertexPtr>& selection)
	: m_cam(cam)
	, m_vp(vp)
	, m_sub_mgr(sub_mgr)
	, m_selected(selected)
	, m_selection(selection)
{
	m_cam2d.OnSize(static_cast<int>(m_vp.Width()), static_cast<int>(m_vp.Height()));

	m_last_pos.MakeInvalid();
}

bool VertexTranslateOP::OnMouseLeftDown(int x, int y)
{
	if (ee0::EditOP::OnMouseLeftDown(x, y)) {
		return true;
	}

	if (m_selection.IsEmpty()) {
		return false;
	}

	m_last_pos.MakeInvalid();

	auto pos = m_cam2d.TransPosScreenToProject(x, y,
		static_cast<int>(m_vp.Width()), static_cast<int>(m_vp.Height()));
	auto cam_mat = m_cam.GetModelViewMat() * m_cam.GetProjectionMat();
	m_selection.Traverse([&](const quake::BrushVertexPtr& vert)->bool 
	{
		auto p3 = vert->pos * model::MapLoader::VERTEX_SCALE;
		auto p2 = m_vp.TransPosProj3ToProj2(p3, cam_mat);
		if (sm::dis_pos_to_pos(p2, pos) < NODE_QUERY_RADIUS) {
			m_last_pos = p3;
			return false;
		} else {
			return true;
		}
	});

	return false;
}

bool VertexTranslateOP::OnMouseLeftUp(int x, int y)
{
	if (ee0::EditOP::OnMouseLeftUp(x, y)) {
		return true;
	}

	m_last_pos.MakeInvalid();

	return false;
}

bool VertexTranslateOP::OnMouseDrag(int x, int y)
{
	if (ee0::EditOP::OnMouseDrag(x, y)) {
		return true;
	}
	
	if (!m_last_pos.IsValid()) {
		return false;
	}

	sm::vec3 ray_dir = m_vp.TransPos3ScreenToDir(
		sm::vec2(static_cast<float>(x), static_cast<float>(y)), m_cam);
	sm::Ray ray(m_cam.GetPos(), ray_dir);
	
	sm::Plane plane(sm::vec3(0, 1, 0), -m_last_pos.y);
	sm::vec3 cross;
	if (!sm::ray_plane_intersect(ray, plane, &cross)) {
		return false;
	}

	TranslateSelected(cross - m_last_pos);
	m_last_pos = cross;

	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

	return false;
}

void VertexTranslateOP::TranslateSelected(const sm::vec3& offset)
{
	m_selection.Traverse([&](const quake::BrushVertexPtr& vert)->bool
	{
		vert->pos += offset / model::MapLoader::VERTEX_SCALE;
		return true;
	});

	// update vbo
	model::MapLoader::UpdateVBO(*m_selected.model, m_selected.brush_idx);
}

}
}