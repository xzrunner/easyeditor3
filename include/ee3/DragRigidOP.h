#pragma once

#include <ee0/EditOP.h>

#include <SM_Vector.h>
#include <uniphysics/rigid/World.h>

namespace ee0 { class WxStagePage; }
namespace pt3 { class PerspCam; class Viewport; }
namespace up
{
namespace rigid
{
    class Body;
    class Constraint;
    class World;
}
}

namespace ee3
{

class DragRigidOP : public ee0::EditOP
{
public:
    DragRigidOP(const std::shared_ptr<pt0::Camera>& camera,
        ee0::WxStagePage& stage, const pt3::Viewport& vp,
        const std::shared_ptr<up::rigid::World>& world);

    virtual bool OnMouseLeftDown(int x, int y) override;
    virtual bool OnMouseLeftUp(int x, int y) override;
    virtual bool OnMouseDrag(int x, int y) override;

private:
    sm::vec3 GetRayTo(const pt3::PerspCam& cam, int x, int y) const;

    void PickBody(const sm::vec3& ray_from, const sm::vec3& ray_to) const;

private:
    const pt3::Viewport& m_vp;

    std::shared_ptr<up::rigid::World> m_world = nullptr;

    mutable up::rigid::World::Picked m_picked;

}; // DragRigidOP

}