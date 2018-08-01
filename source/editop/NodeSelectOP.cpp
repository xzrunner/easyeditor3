#include "ee3/NodeSelectOP.h"
#include "ee3/WxStageCanvas.h"

#include <ee0/MessageID.h>
#include <ee0/color_config.h>
#include <ee0/WxStagePage.h>

#include <guard/check.h>
#include <SM_Ray.h>
#include <SM_RayIntersect.h>
#include <painting3/PrimitiveDraw.h>
#include <painting3/PerspCam.h>
#ifndef GAME_OBJ_ECS
#include <node0/SceneNode.h>
#include <node3/CompAABB.h>
#include <node3/CompTransform.h>
#endif // GAME_OBJ_ECS

namespace ee3
{

NodeSelectOP::NodeSelectOP(ee0::WxStagePage& stage, pt3::PerspCam& cam,
	                       const pt3::Viewport& vp)
	: ee0::NodeSelectOP(stage)
	, m_cam(cam)
	, m_vp(vp)
{
}

bool NodeSelectOP::OnDraw() const
{
	if (ee0::NodeSelectOP::OnDraw()) {
		return true;
	}

	m_stage.GetSelection().Traverse([](const ee0::GameObjWithPos& nwp)->bool
	{
		pt3::PrimitiveDraw::SetColor(ee0::MID_RED.ToABGR());

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

		pt3::PrimitiveDraw::Cube(prev_mt * ctrans.GetTransformMat(), caabb.GetAABB());
#endif // GAME_OBJ_ECS

		return true;
	});

	return false;
}

// AABB not changed, transform ray from Camera and spr's mat
ee0::GameObj NodeSelectOP::QueryByPos(int screen_x, int screen_y) const
{
	sm::vec3 ray_dir = m_vp.TransPos3ScreenToDir(sm::vec2(screen_x, screen_y), m_cam);
	sm::Ray ray(m_cam.GetPos(), ray_dir);

	ee0::VariantSet vars;
	ee0::Variant var;
	var.m_type = ee0::VT_LONG;
	var.m_val.l = ee0::WxStagePage::TRAV_QUERY;
	vars.SetVariant("type", var);

#ifndef GAME_OBJ_ECS
	ee0::GameObj ret = nullptr;
	m_stage.Traverse([&](const ee0::GameObj& obj)->bool
	{
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