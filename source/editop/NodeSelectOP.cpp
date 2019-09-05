#include "ee3/NodeSelectOP.h"
#include "ee3/WxStageCanvas.h"

#include <ee0/MessageID.h>
#include <ee0/color_config.h>
#include <ee0/WxStagePage.h>

#include <guard/check.h>
#include <SM_Ray.h>
#include <SM_RayIntersect.h>
#include <tessellation/Painter.h>
#include <painting2/RenderSystem.h>
#include <painting3/PerspCam.h>
#ifndef GAME_OBJ_ECS
#include <node0/SceneNode.h>
#include <node0/NodeFlagsHelper.h>
#include <node0/NodeFlags.h>
#include <node3/CompAABB.h>
#include <node3/CompTransform.h>
#endif // GAME_OBJ_ECS

namespace ee3
{

NodeSelectOP::NodeSelectOP(const std::shared_ptr<pt0::Camera>& camera,
	                       ee0::WxStagePage& stage, const pt3::Viewport& vp)
	: ee0::NodeSelectOP(camera, stage)
	, m_vp(vp)
{
}

bool NodeSelectOP::OnDraw() const
{
	if (ee0::NodeSelectOP::OnDraw()) {
		return true;
	}

	tess::Painter pt;

	auto cam_mat = m_camera->GetProjectionMat() * m_camera->GetViewMat();
	m_stage.GetSelection().Traverse([&](const ee0::GameObjWithPos& nwp)->bool
	{
#ifndef GAME_OBJ_ECS
		auto& caabb = nwp.GetNode()->GetUniqueComp<n3::CompAABB>();
		auto& ctrans = nwp.GetNode()->GetUniqueComp<n3::CompTransform>();

		sm::mat4 prev_mt;
		// todo
		//auto parent = obj->GetParent();
		//while (parent)
		//{
		//	auto& pctrans = parent->GetUniqueComp<n3::CompTransform>();
		//	prev_mt = pctrans.GetTransformMat() * prev_mt;
		//	parent = parent->GetParent();
		//}

		pt.AddCube(caabb.GetAABB().Cube(), [&](const sm::vec3& pos3)->sm::vec2 {
			auto fix = prev_mt * ctrans.GetTransformMat() * pos3;
			return m_vp.TransPosProj3ToProj2(fix, cam_mat);
		}, ee0::MID_RED.ToABGR());
#endif // GAME_OBJ_ECS

		return true;
	});

	pt2::RenderSystem::DrawPainter(pt);

	return false;
}

// AABB not changed, transform ray from Camera and spr's mat
ee0::GameObj NodeSelectOP::QueryByPos(int screen_x, int screen_y) const
{
	assert(m_camera->TypeID() == pt0::GetCamTypeID<pt3::PerspCam>());
	auto& camera = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);
	sm::vec3 ray_dir = m_vp.TransPos3ScreenToDir(sm::vec2(screen_x, screen_y), *camera);
	sm::Ray ray(camera->GetPos(), ray_dir);

	ee0::VariantSet vars;
	ee0::Variant var;
	var.m_type = ee0::VT_LONG;
	var.m_val.l = ee0::WxStagePage::TRAV_QUERY;
	vars.SetVariant("type", var);

#ifndef GAME_OBJ_ECS
	ee0::GameObj ret = nullptr;
	m_stage.Traverse([&](const ee0::GameObj& obj)->bool
	{
        if (n0::NodeFlagsHelper::GetFlag<n0::NodeNotVisible>(*obj)) {
            return true;
        }

		auto& caabb = obj->GetUniqueComp<n3::CompAABB>();
		auto& ctrans = obj->GetUniqueComp<n3::CompTransform>();

		sm::vec3 cross;
		bool intersect = sm::ray_obb_intersect(
			caabb.GetAABB().Cube(),
			ctrans.GetPosition(),
			ctrans.GetAngle(),
			ctrans.GetScale(),
			ray,
			&cross);
		if (intersect) {
			ret = obj;
			return false;
		} else {
			return true;
		}
	}, vars);

	return ret;
#else
	return ee0::GameObj();
#endif // GAME_OBJ_ECS
}

}