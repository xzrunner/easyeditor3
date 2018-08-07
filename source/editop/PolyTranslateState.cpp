#include "ee3/PolyTranslateState.h"
#include "ee3/MeshRayIntersect.h"

#include <ee0/MessageID.h>
#include <ee0/SubjectMgr.h>

#include <SM_RayIntersect.h>
#include <painting3/PerspCam.h>
#include <painting3/OrthoCam.h>
#include <painting3/Viewport.h>
#include <painting3/PrimitiveDraw.h>
#include <model/MapLoader.h>
#include <model/QuakeMapEntity.h>
#include <model/Model.h>

#include <wx/utils.h>

namespace ee3
{
namespace mesh
{

PolyTranslateState::PolyTranslateState(const std::shared_ptr<pt0::Camera>& camera, const pt3::Viewport& vp,
	                                   const ee0::SubjectMgrPtr& sub_mgr, const MeshPointQuery::Selected& selected,
	                                   std::function<void()> update_cb)
	: ee0::EditOpState(camera)
	, m_vp(vp)
	, m_sub_mgr(sub_mgr)
	, m_selected(selected)
	, m_update_cb(update_cb)
	, m_move_fixed_xz(true)
	, m_move_type(MOVE_ANY)
{
	m_first_pos3.MakeInvalid();
	m_last_pos3.MakeInvalid();
	m_first_pos2.MakeInvalid();
	m_last_pos2.MakeInvalid();
}

bool PolyTranslateState::OnKeyPress(int key_code)
{
	switch (key_code)
	{
	case WXK_ALT:
		// Fixed on Y
		if (m_move_fixed_xz) {
			m_move_type = MOVE_Y;
		}
		break;
	case WXK_SHIFT:
		// Axis Restriction
		{
			// already pressed
			if (!m_first_pos3.IsValid()) {
				return false;
			}

			// only for move any type
			if (m_move_type != MOVE_ANY) {
				return false;
			}

			auto cam_type = m_camera->TypeID();
			if (cam_type == pt0::GetCamTypeID<pt3::PerspCam>())
			{
				assert(m_last_pos3.IsValid());
				sm::vec3 d = m_last_pos3 - m_first_pos3;
				sm::vec3 fixed_offset;
				if (fabs(d.x) >= fabs(d.y) && fabs(d.x) >= fabs(d.z)) {
					m_move_type = MOVE_X;
					fixed_offset.x = d.x;
				} else if (fabs(d.y) >= fabs(d.x) && fabs(d.y) >= fabs(d.z)) {
					m_move_type = MOVE_Y;
					fixed_offset.y = d.y;
				} else {
					m_move_type = MOVE_Z;
					fixed_offset.z = d.z;
				}
				fixed_offset += m_first_pos3 - m_last_pos3;
				TranslateSelected(fixed_offset);
				m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
			}
			else if (cam_type == pt0::GetCamTypeID<pt3::OrthoCam>())
			{
				auto o_cam = std::dynamic_pointer_cast<pt3::OrthoCam>(m_camera);
				auto vp = o_cam->GetViewPlaneType();
				auto first = o_cam->TransPosScreenToProject(m_first_pos2.x, m_first_pos2.y);
				auto last  = o_cam->TransPosScreenToProject(m_last_pos2.x, m_last_pos2.y);
				auto d = last - first;
				sm::vec3 fixed_offset;
				switch (vp)
				{
				case pt3::OrthoCam::VP_ZY:
					if (fabs(d.x) > fabs(d.y))
					{
						m_move_type = MOVE_Z;
						fixed_offset.z = d.x;
					}
					else
					{
						m_move_type = MOVE_Y;
						fixed_offset.y = d.y;
					}
					fixed_offset += sm::vec3(0, -d.y, -d.x);
					break;
				case pt3::OrthoCam::VP_XZ:
					if (fabs(d.x) > fabs(d.y))
					{
						m_move_type = MOVE_X;
						fixed_offset.x = d.x;
					}
					else
					{
						m_move_type = MOVE_Z;
						fixed_offset.z = d.y;
					}
					fixed_offset += sm::vec3(-d.x, 0, -d.y);
					break;
				case pt3::OrthoCam::VP_XY:
					if (fabs(d.x) > fabs(d.y))
					{
						m_move_type = MOVE_X;
						fixed_offset.x = d.x;
					}
					else
					{
						m_move_type = MOVE_Y;
						fixed_offset.y = d.y;
					}
					fixed_offset += sm::vec3(-d.x, -d.y, 0);
					break;
				}
				TranslateSelected(fixed_offset);
				m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
			}
		}
		break;
	}

	return false;
}

bool PolyTranslateState::OnKeyRelease(int key_code)
{
	switch (key_code)
	{
	case WXK_ALT:
		// Fixed on Y
		if (m_move_fixed_xz) {
			m_move_type = MOVE_ANY;
		}
		break;
	case WXK_SHIFT:
		// Axis Restriction
		{
			// not axis restriction
			if (m_move_type == MOVE_ANY) {
				return false;
			}

			auto cam_type = m_camera->TypeID();
			if (cam_type == pt0::GetCamTypeID<pt3::PerspCam>())
			{
				sm::vec3 fixed_offset;
				switch (m_move_type)
				{
				case MOVE_X:
					fixed_offset.x = m_first_pos3.x - m_last_pos3.x;
					break;
				case MOVE_Y:
					fixed_offset.y = m_first_pos3.y - m_last_pos3.y;
					break;
				case MOVE_Z:
					fixed_offset.z = m_first_pos3.z - m_last_pos3.z;
					break;
				}
				fixed_offset += m_last_pos3 - m_first_pos3;
				TranslateSelected(fixed_offset);
			}
			else if (cam_type == pt0::GetCamTypeID<pt3::OrthoCam>())
			{
				auto o_cam = std::dynamic_pointer_cast<pt3::OrthoCam>(m_camera);
				auto vp = o_cam->GetViewPlaneType();
				auto first = o_cam->TransPosScreenToProject(m_first_pos2.x, m_first_pos2.y);
				auto last = o_cam->TransPosScreenToProject(m_last_pos2.x, m_last_pos2.y);
				auto d = last - first;
				sm::vec3 fixed_offset;
				switch (m_move_type)
				{
				case MOVE_X:
					switch (vp)
					{
					case pt3::OrthoCam::VP_XZ:
						fixed_offset.x = -d.x;
						fixed_offset += sm::vec3(d.x, 0, d.y);
						break;
					case pt3::OrthoCam::VP_XY:
						fixed_offset.x = -d.x;
						fixed_offset += sm::vec3(d.x, d.y, 0);
						break;
					}
					break;
				case MOVE_Y:
					switch (vp)
					{
					case pt3::OrthoCam::VP_ZY:
						fixed_offset.y = -d.x;
						fixed_offset += sm::vec3(0, d.y, d.x);
						break;
					case pt3::OrthoCam::VP_XY:
						fixed_offset.y = -d.y;
						fixed_offset += sm::vec3(d.x, d.y, 0);
						break;
					}
					break;
				case MOVE_Z:
					switch (vp)
					{
					case pt3::OrthoCam::VP_ZY:
						fixed_offset.z = -d.y;
						fixed_offset += sm::vec3(0, d.y, d.x);
						break;
					case pt3::OrthoCam::VP_XZ:
						fixed_offset.z = -d.y;
						fixed_offset += sm::vec3(d.x, 0, d.y);
						break;
					}
					break;
				}
				TranslateSelected(fixed_offset);
			}
			m_move_type = MOVE_ANY;
		}
		break;
	}

	return false;
}

bool PolyTranslateState::OnMousePress(int x, int y)
{
	m_first_pos3 = m_selected.pos;
	m_last_pos3  = m_first_pos3;

	m_first_pos2.Set(x, y);
	m_last_pos2 = m_first_pos2;

	return false;
}

bool PolyTranslateState::OnMouseRelease(int x, int y)
{
	m_first_pos3.MakeInvalid();
	m_last_pos3.MakeInvalid();
	m_first_pos2.MakeInvalid();
	m_last_pos2.MakeInvalid();

//	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

	return false;
}

bool PolyTranslateState::OnMouseDrag(int x, int y)
{
	if (!m_last_pos3.IsValid()) {
		return false;
	}

	auto cam_type = m_camera->TypeID();
	if (cam_type == pt0::GetCamTypeID<pt3::PerspCam>())
	{
		sm::vec3 offset;
		auto p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);
		if (m_move_fixed_xz && m_move_type == MOVE_Y)
		{
			sm::vec3 cross;
			if (ray_yline_intersect(*p_cam, m_vp, m_last_pos3, sm::ivec2(x, y), cross)) {
				offset = cross - m_last_pos3;
				m_last_pos3 = cross;
			}
		}
		else
		{
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

			offset = cross - m_last_pos3;
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

			m_last_pos3 = cross;
		}

		TranslateSelected(offset);

		m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
	}
	else if (cam_type == pt0::GetCamTypeID<pt3::OrthoCam>())
	{
		auto o_cam = std::dynamic_pointer_cast<pt3::OrthoCam>(m_camera);

		auto vp = o_cam->GetViewPlaneType();
		auto old_pos = o_cam->TransPosScreenToProject(m_last_pos2.x, m_last_pos2.y);
		auto new_pos = o_cam->TransPosScreenToProject(x, y);
		auto d_pos = new_pos - old_pos;
		switch (vp)
		{
		case pt3::OrthoCam::VP_ZY:
			switch (m_move_type)
			{
			case MOVE_ANY:
				TranslateSelected(sm::vec3(0, d_pos.y, d_pos.x));
				break;
			case MOVE_X:
				assert(0);
				break;
			case MOVE_Y:
				TranslateSelected(sm::vec3(0, d_pos.y, 0));
				break;
			case MOVE_Z:
				TranslateSelected(sm::vec3(0, 0, d_pos.x));
				break;
			}
			break;
		case pt3::OrthoCam::VP_XZ:
			switch (m_move_type)
			{
			case MOVE_ANY:
				TranslateSelected(sm::vec3(d_pos.x, 0, d_pos.y));
				break;
			case MOVE_X:
				TranslateSelected(sm::vec3(d_pos.x, 0, 0));
				break;
			case MOVE_Y:
				assert(0);
				break;
			case MOVE_Z:
				TranslateSelected(sm::vec3(0, 0, d_pos.y));
				break;
			}
			break;
		case pt3::OrthoCam::VP_XY:
			switch (m_move_type)
			{
			case MOVE_ANY:
				TranslateSelected(sm::vec3(d_pos.x, d_pos.y, 0));
				break;
			case MOVE_X:
				TranslateSelected(sm::vec3(d_pos.x, 0, 0));
				break;
			case MOVE_Y:
				TranslateSelected(sm::vec3(0, d_pos.y, 0));
				break;
			case MOVE_Z:
				assert(0);
				break;
			}
			break;
		}

		m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
	}

	m_last_pos2.Set(x, y);

	return false;
}

bool PolyTranslateState::OnDraw() const
{
	// draw auxiliary line
	auto cam_type = m_camera->TypeID();
	if (cam_type == pt0::GetCamTypeID<pt3::PerspCam>())
	{
		if (m_first_pos3.IsValid() && m_last_pos3.IsValid())
		{
			auto& first = m_first_pos3;
			auto last = m_last_pos3;
			switch (m_move_type)
			{
			case MOVE_ANY:
				pt3::PrimitiveDraw::SetColor(0x0ff0000ff);
				pt3::PrimitiveDraw::Line(first, sm::vec3(last.x, first.y, first.z));
				pt3::PrimitiveDraw::SetColor(0x0ff00ff00);
				pt3::PrimitiveDraw::Line(sm::vec3(last.x, first.y, first.z), sm::vec3(last.x, last.y, first.z));
				pt3::PrimitiveDraw::SetColor(0x0ffff0000);
				pt3::PrimitiveDraw::Line(sm::vec3(last.x, last.y, first.z), m_last_pos3);
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
	}
	else if (cam_type == pt0::GetCamTypeID<pt3::OrthoCam>())
	{
		if (m_first_pos2.IsValid() && m_last_pos2.IsValid())
		{
			auto o_cam = std::dynamic_pointer_cast<pt3::OrthoCam>(m_camera);
			auto first = o_cam->TransPosScreenToProject(m_first_pos2.x, m_first_pos2.y);
			auto last  = o_cam->TransPosScreenToProject(m_last_pos2.x, m_last_pos2.y);
			auto vp = o_cam->GetViewPlaneType();
			switch (vp)
			{
			case pt3::OrthoCam::VP_ZY:
				switch (m_move_type)
				{
				case MOVE_ANY:
					pt3::PrimitiveDraw::SetColor(0x0ff00ff00);
					pt3::PrimitiveDraw::Line(sm::vec3(0, first.y, first.x), sm::vec3(0, last.y, first.x));
					pt3::PrimitiveDraw::SetColor(0x0ffff0000);
					pt3::PrimitiveDraw::Line(sm::vec3(0, last.y, first.x), sm::vec3(0, last.y, last.x));
					break;
				case MOVE_Z:
					pt3::PrimitiveDraw::SetColor(0x0ffff0000);
					pt3::PrimitiveDraw::Line(sm::vec3(0, first.y, first.x), sm::vec3(0, first.y, last.x));
					break;
				case MOVE_Y:
					pt3::PrimitiveDraw::SetColor(0x0ff00ff00);
					pt3::PrimitiveDraw::Line(sm::vec3(0, first.y, first.x), sm::vec3(0, last.y, first.x));
					break;
				}
				break;
			case pt3::OrthoCam::VP_XZ:
				switch (m_move_type)
				{
				case MOVE_ANY:
					pt3::PrimitiveDraw::SetColor(0x0ff0000ff);
					pt3::PrimitiveDraw::Line(sm::vec3(first.x, 0, first.y), sm::vec3(last.x, 0, first.y));
					pt3::PrimitiveDraw::SetColor(0x0ffff0000);
					pt3::PrimitiveDraw::Line(sm::vec3(last.x, 0, first.y), sm::vec3(last.x, 0, last.y));
					break;
				case MOVE_X:
					pt3::PrimitiveDraw::SetColor(0x0ff0000ff);
					pt3::PrimitiveDraw::Line(sm::vec3(first.x, 0, first.y), sm::vec3(last.x, 0, first.y));
					break;
				case MOVE_Z:
					pt3::PrimitiveDraw::SetColor(0x0ffff0000);
					pt3::PrimitiveDraw::Line(sm::vec3(first.x, 0, first.y), sm::vec3(first.x, 0, last.y));
					break;
				}
				break;
			case pt3::OrthoCam::VP_XY:
				switch (m_move_type)
				{
				case MOVE_ANY:
					pt3::PrimitiveDraw::SetColor(0x0ff0000ff);
					pt3::PrimitiveDraw::Line(sm::vec3(first.x, first.y, 0), sm::vec3(last.x, first.y, 0));
					pt3::PrimitiveDraw::SetColor(0x0ff00ff00);
					pt3::PrimitiveDraw::Line(sm::vec3(last.x, first.y, 0), sm::vec3(last.x, last.y, 0));
					break;
				case MOVE_X:
					pt3::PrimitiveDraw::SetColor(0x0ff0000ff);
					pt3::PrimitiveDraw::Line(sm::vec3(first.x, first.y, 0), sm::vec3(last.x, first.y, 0));
					break;
				case MOVE_Y:
					pt3::PrimitiveDraw::SetColor(0x0ff00ff00);
					pt3::PrimitiveDraw::Line(sm::vec3(first.x, first.y, 0), sm::vec3(first.x, last.y, 0));
					break;
				}
				break;
			}
		}
	}

	return false;
}

bool PolyTranslateState::IsMoveAnyDirection() const
{
	return m_move_type == MOVE_ANY;
}

void PolyTranslateState::CalcTranslatePlane(const sm::Ray& ray, sm::Plane& plane) const
{
	if (m_move_fixed_xz) 
	{
		float doty = ray.dir.Dot(sm::vec3(0, 1, 0));
		if (doty > 0) {
			plane.normal = sm::vec3(0, -1, 0);
			plane.dist = m_last_pos3.y;
		}
		else {
			plane.normal = sm::vec3(0, 1, 0);
			plane.dist = -m_last_pos3.y;
		}
	}
	else
	{
		float dotx = ray.dir.Dot(sm::vec3(1, 0, 0));
		float doty = ray.dir.Dot(sm::vec3(0, 1, 0));
		float dotz = ray.dir.Dot(sm::vec3(0, 0, 1));
		if (fabs(dotx) > fabs(doty) && fabs(dotx) > fabs(dotz))
		{
			if (dotx > 0) {
				plane.normal = sm::vec3(-1, 0, 0);
				plane.dist = m_last_pos3.x;
			} else {
				plane.normal = sm::vec3(1, 0, 0);
				plane.dist = -m_last_pos3.x;
			}
		}
		else if (fabs(doty) > fabs(dotx) && fabs(doty) > fabs(dotz))
		{
			if (doty > 0) {
				plane.normal = sm::vec3(0, -1, 0);
				plane.dist = m_last_pos3.y;
			} else {
				plane.normal = sm::vec3(0, 1, 0);
				plane.dist = -m_last_pos3.y;
			}
		}
		else
		{
			if (dotz > 0) {
				plane.normal = sm::vec3(0, 0, -1);
				plane.dist = m_last_pos3.z;
			} else {
				plane.normal = sm::vec3(0, 0, 1);
				plane.dist = -m_last_pos3.z;
			}
		}
	}
}

void PolyTranslateState::TranslateSelected(const sm::vec3& offset)
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