#include "ee3/PolyArrangeOP.h"
#include "ee3/FacePushPullState.h"

#include <ee0/MessageID.h>
#include <ee0/SubjectMgr.h>

#include <SM_RayIntersect.h>
#include <painting3/PerspCam.h>
#include <painting3/Viewport.h>
#include <model/MapLoader.h>
#include <model/QuakeMapEntity.h>
#include <model/Model.h>

#include <wx/defs.h>

namespace ee3
{

PolyArrangeOP::PolyArrangeOP(pt3::PerspCam& cam,
	                         const pt3::Viewport& vp,
	                         const ee0::SubjectMgrPtr& sub_mgr,
	                         const MeshPointQuery::Selected& m_selected)
	: m_cam(cam)
	, m_vp(vp)
	, m_sub_mgr(sub_mgr)
	, m_selected(m_selected)
{
	m_last_pos.MakeInvalid();
}

bool PolyArrangeOP::OnKeyDown(int key_code)
{
	if (ee0::EditOP::OnKeyDown(key_code)) {
		return true;
	}

	if (!m_face_pp_state)
	{
		if (key_code == WXK_SHIFT && m_selected.poly) {
			m_face_pp_state = std::make_shared<FacePushPullState>(m_cam, m_vp, m_sub_mgr, m_selected);
		}
	}

	return false;
}

bool PolyArrangeOP::OnKeyUp(int key_code)
{
	if (ee0::EditOP::OnKeyUp(key_code)) {
		return true;
	}

	if (m_face_pp_state)
	{
		if (key_code == WXK_SHIFT) {
			m_face_pp_state.reset();
//			m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
		}
	}

	return false;
}

bool PolyArrangeOP::OnMouseLeftDown(int x, int y)
{
	if (ee0::EditOP::OnMouseLeftDown(x, y)) {
		return true;
	}
	if (m_face_pp_state && m_face_pp_state->OnMousePress(x, y)) {
		return true;
	}

	m_last_pos = m_selected.pos;

	return false;
}

bool PolyArrangeOP::OnMouseLeftUp(int x, int y)
{
	if (ee0::EditOP::OnMouseLeftUp(x, y)) {
		return true;
	}

	m_last_pos.MakeInvalid();

	return false;
}

bool PolyArrangeOP::OnMouseDrag(int x, int y)
{
	if (ee0::EditOP::OnMouseDrag(x, y)) {
		return true;
	}
	if (m_face_pp_state && m_face_pp_state->OnMouseDrag(x, y)) {
		return true;
	}

	if (!m_last_pos.IsValid()) {
		return false;
	}

	sm::vec3 ray_dir = m_vp.TransPos3ScreenToDir(
		sm::vec2(static_cast<float>(x), static_cast<float>(y)), m_cam);
	sm::Ray ray(m_cam.GetPos(), ray_dir);

	sm::Plane plane;
	CalcTranslatePlane(ray, plane);

	sm::vec3 cross;
	if (!sm::ray_plane_intersect(ray, plane, &cross)) {
		return true;
	}

	sm::vec3 offset = cross - m_last_pos;
	m_last_pos = cross;

	TranslateSelected(offset);

	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

	return false;
}

bool PolyArrangeOP::OnDraw() const
{
	if (ee0::EditOP::OnDraw()) {
		return true;
	}
	if (m_face_pp_state && m_face_pp_state->OnDraw()) {
		return true;
	}
	return false;
}

void PolyArrangeOP::CalcTranslatePlane(const sm::Ray& ray, sm::Plane& plane) const
{
	float dotx = ray.dir.Dot(sm::vec3(1, 0, 0));
	float doty = ray.dir.Dot(sm::vec3(0, 1, 0));
	float dotz = ray.dir.Dot(sm::vec3(0, 0, 1));
	if (fabs(dotx) > fabs(doty) && fabs(dotx) > fabs(dotz))
	{
		if (dotx > 0) {
			plane.normal = sm::vec3(-1, 0, 0);
			plane.dist = m_last_pos.x;
		} else {
			plane.normal = sm::vec3(1, 0, 0);
			plane.dist = -m_last_pos.x;
		}
	}
	else if (fabs(doty) > fabs(dotx) && fabs(doty) > fabs(dotz))
	{
		if (doty > 0) {
			plane.normal = sm::vec3(0, -1, 0);
			plane.dist = m_last_pos.y;
		} else {
			plane.normal = sm::vec3(0, 1, 0);
			plane.dist = -m_last_pos.y;
		}
	}
	else
	{
		if (dotz > 0) {
			plane.normal = sm::vec3(0, 0, -1);
			plane.dist = m_last_pos.z;
		} else {
			plane.normal = sm::vec3(0, 0, 1);
			plane.dist = -m_last_pos.z;
		}
	}
}

void PolyArrangeOP::TranslateSelected(const sm::vec3& offset)
{
	if (!m_selected.poly) {
		return;
	}

	// update quake map brush
	assert(m_selected.model->ext && m_selected.model->ext->Type() == model::EXT_QUAKE_MAP);
	auto map_entity = static_cast<model::QuakeMapEntity*>(m_selected.model->ext.get());
	auto& brushes = map_entity->GetMapEntity()->brushes;
	assert(m_selected.brush_idx >= 0 && m_selected.brush_idx < brushes.size());
	auto& brush = brushes[m_selected.brush_idx];
	for (auto& vert : brush.vertices) {
		vert->pos += offset / model::MapLoader::VERTEX_SCALE;
	}

	// update helfedge geo
	auto& vertices = m_selected.poly->GetVertices();
	for (auto& vert : vertices) {
		vert->position += offset;
	}
	m_selected.poly->UpdateAABB();

	// update model aabb
	sm::cube model_aabb;
	for (auto& brush : brushes) {
		model_aabb.Combine(brush.geometry->GetAABB());
	}
	m_selected.model->aabb = model_aabb;

	//// update m_selected border
	//UpdatePolyBorderPos();

	// update vbo
	model::MapLoader::UpdateVBO(*m_selected.model, m_selected.brush_idx);
}

}