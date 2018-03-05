#include "ee3/NodeSelectOP.h"
#include "ee3/WxStageCanvas.h"

#include <ee0/MessageID.h>
#include <ee0/color_config.h>
#include <ee0/WxStagePage.h>

#include <guard/check.h>
#include <painting3/Ray.h>
#include <painting3/PrimitiveDraw.h>
#include <node0/SceneNode.h>
#include <node3/Math.h>
#include <node3/CompAABB.h>
#include <node3/CompTransform.h>

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

	m_stage.GetNodeSelection().Traverse([](const n0::SceneNodePtr& node)->bool
	{
		pt3::PrimitiveDraw::SetColor(ee0::MID_RED.ToABGR());

		auto& caabb = node->GetUniqueComp<n3::CompAABB>();
		auto& ctrans = node->GetUniqueComp<n3::CompTransform>();

		sm::mat4 prev_mt;
		// todo
		//auto parent = node->GetParent();
		//while (parent)
		//{
		//	auto& pctrans = parent->GetUniqueComp<n3::CompTransform>();
		//	prev_mt = pctrans.GetTransformMat() * prev_mt;
		//	parent = parent->GetParent();
		//}

		pt3::PrimitiveDraw::Cube(prev_mt * ctrans.GetTransformMat(), caabb.GetAABB());

		return true;
	});

	return false;
}

// AABB not changed, transform ray from Camera and spr's mat
n0::SceneNodePtr NodeSelectOP::QueryByPos(int screen_x, int screen_y) const
{
	auto canvas = std::dynamic_pointer_cast<WxStageCanvas>(m_stage.GetImpl().GetCanvas());
	auto& vp = canvas->GetViewport();
	auto& cam = canvas->GetCamera();
	sm::vec3 ray_dir = vp.TransPos3ScreenToDir(sm::vec2(screen_x, screen_y), cam);
	pt3::Ray ray(cam.GetPos(), ray_dir);

	n0::SceneNodePtr ret = nullptr;
	m_stage.Traverse([&](const n0::SceneNodePtr& node)->bool
	{
		auto& caabb = node->GetUniqueComp<n3::CompAABB>();
		auto& ctrans = node->GetUniqueComp<n3::CompTransform>();

		sm::vec3 cross;
		bool intersect = n3::Math::RayOBBIntersection(
			caabb.GetAABB(),
			ctrans.GetPosition(),
			ctrans.GetAngle(),
			ray,
			&cross);
		if (intersect) {
			ret = node;
			return false;
		} else {
			return true;
		}
	});

	return ret;
}

}