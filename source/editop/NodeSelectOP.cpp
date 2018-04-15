#include "ee3/NodeSelectOP.h"
#include "ee3/WxStageCanvas.h"

#include <ee0/MessageID.h>
#include <ee0/color_config.h>
#include <ee0/WxStagePage.h>

#include <guard/check.h>
#include <painting3/Ray.h>
#include <painting3/PrimitiveDraw.h>
#ifndef GAME_OBJ_ECS
#include <node0/SceneNode.h>
#include <node3/Math.h>
#include <node3/CompAABB.h>
#include <node3/CompTransform.h>
#endif // GAME_OBJ_ECS

namespace ee3
{

NodeSelectOP::NodeSelectOP(ee0::WxStagePage& stage)
	: ee0::NodeSelectOP(stage)
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
	auto canvas = std::dynamic_pointer_cast<WxStageCanvas>(m_stage.GetImpl().GetCanvas());
	auto& vp = canvas->GetViewport();
	auto& cam = canvas->GetCamera();
	sm::vec3 ray_dir = vp.TransPos3ScreenToDir(sm::vec2(screen_x, screen_y), cam);
	pt3::Ray ray(cam.GetPos(), ray_dir);

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
		bool intersect = n3::Math::RayOBBIntersection(
			caabb.GetAABB(),
			ctrans.GetPosition(),
			ctrans.GetAngle(),
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