#include "ee3/PolyArrangeOP.h"
#include "ee3/FacePushPullState.h"

#include <ee0/MessageID.h>
#include <ee0/SubjectMgr.h>

#include <SM_RayIntersect.h>
#include <painting3/PerspCam.h>
#include <painting3/Viewport.h>
#include <painting3/PrimitiveDraw.h>
#include <model/MapLoader.h>
#include <model/QuakeMapEntity.h>
#include <model/Model.h>

#include <wx/defs.h>

namespace ee3
{
namespace mesh
{

PolyArrangeOP::PolyArrangeOP(const std::shared_ptr<pt0::Camera>& camera,
	                         const pt3::Viewport& vp,
	                         const ee0::SubjectMgrPtr& sub_mgr,
	                         const MeshPointQuery::Selected& m_selected,
	                         std::function<void()> update_cb)
	: ee0::EditOP(camera)
	, m_vp(vp)
	, m_sub_mgr(sub_mgr)
	, m_selected(m_selected)
	, m_update_cb(update_cb)
	, m_move_type(MOVE_ANY)
{
	m_last_pos.MakeInvalid();
}

bool PolyArrangeOP::OnKeyDown(int key_code)
{
	if (ee0::EditOP::OnKeyDown(key_code)) {
		return true;
	}

	if (key_code == WXK_SHIFT)
	{
		// Axis Restriction
		if (m_first_pos.IsValid())
		{
			if (m_move_type == MOVE_ANY)
			{
				assert(m_last_pos.IsValid());
				sm::vec3 d = m_last_pos - m_first_pos;
				sm::vec3 fixed_offset;
				if (fabs(d.x) >= fabs(d.y) && fabs(d.x) >= fabs(d.z)) {
					m_move_type = MOVE_X;
					fixed_offset.x = d.x;
				}
				else if (fabs(d.y) >= fabs(d.x) && fabs(d.y) >= fabs(d.z)) {
					m_move_type = MOVE_Y;
					fixed_offset.y = d.y;
				}
				else {
					m_move_type = MOVE_Z;
					fixed_offset.z = d.z;
				}
				fixed_offset += m_first_pos - m_last_pos;
				TranslateSelected(fixed_offset);
				m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
			}
		}
		// Face push and pull
		else
		{
			if (m_selected.poly)
			{
				assert(m_camera->TypeID() == pt0::GetCamTypeID<pt3::PerspCam>());
				auto p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);
				m_face_pp_state = std::make_shared<FacePushPullState>(
					p_cam, m_vp, m_sub_mgr, m_selected);
			}
		}
	}

	return false;
}

bool PolyArrangeOP::OnKeyUp(int key_code)
{
	if (ee0::EditOP::OnKeyUp(key_code)) {
		return true;
	}

	if (key_code == WXK_SHIFT)
	{
		if (m_move_type != MOVE_ANY)
		{
			sm::vec3 fixed_offset;
			switch (m_move_type)
			{
			case MOVE_X:
				fixed_offset.x = m_first_pos.x - m_last_pos.x;
				break;
			case MOVE_Y:
				fixed_offset.y = m_first_pos.y - m_last_pos.y;
				break;
			case MOVE_Z:
				fixed_offset.z = m_first_pos.z - m_last_pos.z;
				break;
			}
			fixed_offset += m_last_pos - m_first_pos;
			TranslateSelected(fixed_offset);

			m_move_type = MOVE_ANY;
		}
		else
		{
			m_face_pp_state.reset();
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

	m_first_pos = m_selected.pos;
	m_last_pos  = m_first_pos;

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

	if (m_camera->TypeID() == pt0::GetCamTypeID<pt3::PerspCam>())
	{
		auto p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);

		sm::vec3 center_dir = m_vp.TransPos3ScreenToDir(
			sm::vec2(m_vp.Width() * 0.5f, m_vp.Height() * 0.5f), *p_cam);
		sm::Ray center_ray(p_cam->GetPos(), center_dir);

		sm::Plane plane;
		CalcTranslatePlane(center_ray, plane);

		sm::vec3 ray_dir = m_vp.TransPos3ScreenToDir(
			sm::vec2(static_cast<float>(x), static_cast<float>(y)), *p_cam);
		sm::Ray ray(p_cam->GetPos(), ray_dir);

		sm::vec3 cross;
		if (!sm::ray_plane_intersect(ray, plane, &cross)) {
			return true;
		}

		sm::vec3 offset = cross - m_last_pos;
		switch (m_move_type)
		{
		case MOVE_X:
			offset.y = offset.z = 0;
			break;
		case MOVE_Y:
			offset.x = offset.z = 0;
			break;
		case MOVE_Z:
			offset.x = offset.y = 0;
			break;
		}
		TranslateSelected(offset);

		m_last_pos = cross;

		m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
	}

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

	// auxiliary line
	if (m_first_pos.IsValid() && m_last_pos.IsValid())
	{
		auto& first = m_first_pos;
		auto last = m_last_pos;
		switch (m_move_type)
		{
		case MOVE_ANY:
			pt3::PrimitiveDraw::SetColor(0x0ff0000ff);
			pt3::PrimitiveDraw::Line(first, sm::vec3(last.x, first.y, first.z));
			pt3::PrimitiveDraw::SetColor(0x0ff00ff00);
			pt3::PrimitiveDraw::Line(sm::vec3(last.x, first.y, first.z), sm::vec3(last.x, last.y, first.z));
			pt3::PrimitiveDraw::SetColor(0x0ffff0000);
			pt3::PrimitiveDraw::Line(sm::vec3(last.x, last.y, first.z), m_last_pos);
			break;
		case MOVE_X:
			last.y = first.y;
			last.z = first.z;
			pt3::PrimitiveDraw::SetColor(0x0ff0000ff);
			pt3::PrimitiveDraw::Line(first, last);
			break;
		case MOVE_Y:
			last.x = first.x;
			last.z = first.z;
			pt3::PrimitiveDraw::SetColor(0x0ff00ff00);
			pt3::PrimitiveDraw::Line(first, last);
			break;
		case MOVE_Z:
			last.x = first.x;
			last.y = first.y;
			pt3::PrimitiveDraw::SetColor(0x0ffff0000);
			pt3::PrimitiveDraw::Line(first, last);
			break;
		}
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

	// update vbo
	model::MapLoader::UpdateVBO(*m_selected.model, m_selected.brush_idx);

	// update m_selected border
	m_update_cb();
}

}
}