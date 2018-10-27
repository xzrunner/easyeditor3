#include "ee3/FacePushPullState.h"

#include <ee0/color_config.h>
#include <ee0/SubjectMgr.h>
#include <ee0/MessageID.h>

#include <SM_RayIntersect.h>
#include <tessellation/Painter.h>
#include <painting2/OrthoCamera.h>
#include <painting2/RenderSystem.h>
#include <painting3/Viewport.h>
#include <painting3/PerspCam.h>
#include <model/QuakeMapEntity.h>
#include <model/Model.h>
#include <model/MapLoader.h>

namespace ee3
{
namespace mesh
{

FacePushPullState::FacePushPullState(const std::shared_ptr<pt0::Camera>& camera,
	                                 const pt3::Viewport& vp,
	                                 const ee0::SubjectMgrPtr& sub_mgr,
	                                 const MeshPointQuery::Selected& selected)
	: ee0::EditOpState(camera)
	, m_vp(vp)
	, m_sub_mgr(sub_mgr)
	, m_selected(selected)
	, m_cam2d(std::make_shared<pt2::OrthoCamera>())
{
	m_cam2d->OnSize(m_vp.Width(), m_vp.Height());

	m_last_pos3d.MakeInvalid();
}

bool FacePushPullState::OnMousePress(int x, int y)
{
	if (!m_selected.face) {
		return false;
	}

	if (m_camera->TypeID() == pt0::GetCamTypeID<pt3::PerspCam>())
	{
		auto p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);

		m_first_pos2 = m_cam2d->TransPosScreenToProject(x, y,
			static_cast<int>(m_vp.Width()), static_cast<int>(m_vp.Height()));

		sm::vec3 ray_dir = m_vp.TransPos3ScreenToDir(sm::vec2((float)x, (float)y), *p_cam);
		sm::Ray ray(p_cam->GetPos(), ray_dir);
		sm::vec3 intersect;
		sm::Plane plane;
		m_selected.face->GetPlane(plane);
		bool crossed = false;
		if (crossed = sm::ray_plane_intersect(ray, plane, &intersect)) {
			m_move_path3d.origin = intersect;
		}
		assert(crossed);
		m_move_path3d.dir = plane.normal;

		m_cam_mat = m_camera->GetViewMat() * m_camera->GetProjectionMat();
		auto next_pos2 = m_vp.TransPosProj3ToProj2(m_move_path3d.origin + m_move_path3d.dir, m_cam_mat);
		m_first_dir2 = (next_pos2 - m_first_pos2).Normalized();
	}

	return false;
}

bool FacePushPullState::OnMouseDrag(int x, int y)
{
	if (m_camera->TypeID() == pt0::GetCamTypeID<pt3::PerspCam>())
	{
		auto p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);

		auto curr_pos2 = m_cam2d->TransPosScreenToProject(x, y,
			static_cast<int>(m_vp.Width()), static_cast<int>(m_vp.Height()));
		auto fixed_curr_pos2 = m_first_pos2 + m_first_dir2 * ((curr_pos2 - m_first_pos2).Dot(m_first_dir2));
		auto screen_fixed_curr_pos2 = m_cam2d->TransPosProjectToScreen(fixed_curr_pos2,
			static_cast<int>(m_vp.Width()), static_cast<int>(m_vp.Height()));

		sm::vec3 ray_dir = m_vp.TransPos3ScreenToDir(screen_fixed_curr_pos2, *p_cam);
		sm::Ray ray(p_cam->GetPos(), ray_dir);
		sm::vec3 cross;
		if (sm::ray_ray_intersect(ray, m_move_path3d, &cross)) {
			if (m_last_pos3d.IsValid()) {
				TranslateFace(cross - m_last_pos3d);
			}
			m_last_pos3d = cross;
		} else {
			m_last_pos3d.MakeInvalid();
		}

		m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

		return true;
	}
	else
	{
		return false;
	}
}

bool FacePushPullState::OnDraw() const
{
	// debug draw
	if (m_last_pos3d.IsValid())
	{
		auto cam_mat = m_camera->GetViewMat() * m_camera->GetProjectionMat();
		tess::Painter pt;
		pt.AddLine3D(m_move_path3d.origin, m_last_pos3d, [&](const sm::vec3& pos3)->sm::vec2 {
			return m_vp.TransPosProj3ToProj2(pos3, cam_mat);
		}, 0xffffffff);
		pt2::RenderSystem::DrawPainter(pt);
	}

	return false;
}

void FacePushPullState::TranslateFace(const sm::vec3& offset)
{
	if (!m_selected.face) {
		return;
	}

	// quake map brush data
	assert(m_selected.model->ext && m_selected.model->ext->Type() == model::EXT_QUAKE_MAP);
	auto map_entity = static_cast<model::QuakeMapEntity*>(m_selected.model->ext.get());
	auto& brushes = map_entity->GetMapEntity()->brushes;
	assert(m_selected.brush_idx >= 0 && m_selected.brush_idx < static_cast<int>(brushes.size()));
	auto& brush = brushes[m_selected.brush_idx];
	assert(m_selected.face_idx < static_cast<int>(brush.faces.size()));
	auto& face = brush.faces[m_selected.face_idx];
	for (auto& vert : face->vertices) {
		vert->pos += offset / model::MapLoader::VERTEX_SCALE;
	}

	// halfedge geo
	auto start = m_selected.face->start_edge;
	auto ptr = start;
	while (ptr)
	{
		ptr->origin->position += offset;

		ptr = ptr->next;
		if (ptr == start) {
			break;
		}
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
}

}
}