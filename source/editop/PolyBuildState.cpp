#include "ee3/PolyBuildState.h"

#include <ee0/SubjectMgr.h>
#include <ee0/MessageID.h>

#include <SM_Ray.h>
#include <SM_RayIntersect.h>
#include <tessellation/Painter.h>
#include <painting2/RenderSystem.h>
#include <painting3/PerspCam.h>
#include <painting3/Viewport.h>

namespace ee3
{
namespace mesh
{

PolyBuildState::PolyBuildState(const std::shared_ptr<pt0::Camera>& camera,
	                           const pt3::Viewport& vp,
	                           const ee0::SubjectMgrPtr& sub_mgr)
	: ee0::EditOpState(camera)
	, m_vp(vp)
	, m_sub_mgr(sub_mgr)
	, m_y(0)
{
	m_first_pos.MakeInvalid();
}

bool PolyBuildState::OnKeyPress(int key_code)
{
	return false;
}

bool PolyBuildState::OnKeyRelease(int key_code)
{
	return false;
}

bool PolyBuildState::OnMousePress(int x, int y)
{
	sm::vec3 cross;
	if (RayPlaneIntersect(x, y, m_y, cross)) {
		m_first_pos = cross;
	}

	return false;
}

bool PolyBuildState::OnMouseRelease(int x, int y)
{
	return false;
}

bool PolyBuildState::OnMouseDrag(int x, int y)
{
	sm::vec3 cross;
	if (RayPlaneIntersect(x, y, m_y, cross)) {
		m_last_pos = cross;
		m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
	}

	return false;
}

bool PolyBuildState::OnDraw() const
{
	if (m_first_pos.IsValid() && m_last_pos.IsValid())
	{
		tess::Painter pt;
		auto cam_mat = m_camera->GetViewMat() * m_camera->GetProjectionMat();
		pt.AddCube(sm::cube(m_first_pos, m_last_pos), [&](const sm::vec3& pos3)->sm::vec2 {
			return m_vp.TransPosProj3ToProj2(pos3, cam_mat);
		}, 0xffffffff);
		pt2::RenderSystem::DrawPainter(pt);
	}
	return false;
}

bool PolyBuildState::RayPlaneIntersect(int x, int y, float plane_y, sm::vec3& cross) const
{
	if (m_camera->TypeID() != pt0::GetCamTypeID<pt3::PerspCam>()) {
		return false;
	}

	auto p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);

	sm::vec3 ray_dir = m_vp.TransPos3ScreenToDir(
		sm::vec2(static_cast<float>(x), static_cast<float>(y)), *p_cam);
	sm::Ray ray(p_cam->GetPos(), ray_dir);

	sm::Plane plane;
	if (ray_dir.y < 0) {
		plane.Build(sm::vec3(0, 1, 0), -plane_y);
	} else {
		plane.Build(sm::vec3(0, -1, 0), plane_y);
	}

	return sm::ray_plane_intersect(ray, plane, &cross);
}

}
}