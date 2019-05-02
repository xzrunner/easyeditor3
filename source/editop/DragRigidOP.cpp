#include "ee3/DragRigidOP.h"

#include <painting3/PerspCam.h>
#include <painting3/Viewport.h>
#include <uniphysics/rigid/Body.h>

namespace ee3
{

DragRigidOP::DragRigidOP(const std::shared_ptr<pt0::Camera>& camera,
                         ee0::WxStagePage& stage, const pt3::Viewport& vp,
                         const std::shared_ptr<up::rigid::World>& world)
    : ee0::EditOP(camera)
    , m_vp(vp)
    , m_world(world)
{
}

bool DragRigidOP::OnMouseLeftDown(int x, int y)
{
    if (ee0::EditOP::OnMouseLeftDown(x, y)) {
        return true;
    }

    printf("OnMouseLeftDown\n");

    if (m_camera->TypeID() == pt0::GetCamTypeID<pt3::PerspCam>())
    {
        auto p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);

        auto ray_from = p_cam->GetPos();
        auto ray_to = GetRayTo(*p_cam, x, y);
        PickBody(ray_from, ray_to);
    }

    return false;
}

bool DragRigidOP::OnMouseLeftUp(int x, int y)
{
    if (ee0::EditOP::OnMouseLeftUp(x, y)) {
        return true;
    }

    printf("OnMouseLeftUp\n");

    if (m_picked.cons)
    {
        m_picked.body->ForceActivationState(m_picked.saved_state);
        m_picked.body->Activate();
        m_world->RemoveConstraint(m_picked.cons);
        m_picked.Reset();
    }

    return false;
}

bool DragRigidOP::OnMouseDrag(int x, int y)
{
    if (ee0::EditOP::OnMouseDrag(x, y)) {
        return true;
    }

    if (m_picked.body && m_picked.cons)
    {
        if (m_camera->TypeID() == pt0::GetCamTypeID<pt3::PerspCam>())
        {
            auto p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);

            auto ray_from = p_cam->GetPos();
            auto ray_to = GetRayTo(*p_cam, x, y);
            m_world->MovePickedBody(m_picked, ray_from, ray_to);
        }
    }

    return false;
}

sm::vec3 DragRigidOP::GetRayTo(const pt3::PerspCam& cam, int x, int y) const
{
    float top = 1.f;
    float bottom = -1.f;
//    float near_plane = cam.GetNear();
    float near_plane = 1.0f;
    float tan_fov = (top - bottom) * 0.5f / near_plane;
    float fov = 2.0f * std::atan(tan_fov);

    auto pos = cam.GetPos();
    auto target = cam.GetTarget();

    auto ray_from = pos;
    auto ray_forward = target - pos;
    ray_forward.Normalize();
//    float far_plane = cam.GetFar();
    float far_plane = 10000.0f;
    ray_forward *= far_plane;

//	auto& camera_up = cam.GetUpDir();
    sm::vec3 camera_up(0, 1, 0);
//	camera_up[m_guiHelper->getAppInterface()->getUpAxis()] = 1;

	sm::vec3 vert = camera_up;

	sm::vec3 hor;
	hor = ray_forward.Cross(vert);
	hor.Normalize();
	vert = hor.Cross(ray_forward);
	vert.Normalize();

	float tanfov = tanf(0.5f * fov);

	hor  *= 2.f * far_plane * tanfov;
	vert *= 2.f * far_plane * tanfov;

	float aspect;
	float width = m_vp.Width();
	float height = m_vp.Height();

	aspect = width / height;

	hor *= aspect;

	sm::vec3 ray_to_center = ray_from + ray_forward;
	sm::vec3 d_hor  = hor  * 1.f / width;
	sm::vec3 d_vert = vert * 1.f / height;

	sm::vec3 ray_to = ray_to_center - hor * 0.5f + vert * 0.5f;
	ray_to += d_hor  * float(x);
	ray_to -= d_vert * float(y);

    return ray_to;
}

void DragRigidOP::PickBody(const sm::vec3& ray_from, const sm::vec3& ray_to) const
{
    m_picked = m_world->PickBody(ray_from, ray_to);
}

}