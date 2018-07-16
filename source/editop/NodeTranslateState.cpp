#include "ee3/NodeTranslateState.h"

#include <ee0/SubjectMgr.h>
#include <ee0/MessageID.h>

#include <SM_Calc.h>
#include <painting3/Camera.h>
#include <painting3/Viewport.h>
#ifndef GAME_OBJ_ECS
#include <node0/SceneNode.h>
#include <node3/CompTransform.h>
#include <node3/CompAABB.h>
#endif // GAME_OBJ_ECS

namespace ee3
{

NodeTranslateState::NodeTranslateState(const pt3::Camera& cam, const pt3::Viewport& vp,
	                                   const ee0::SubjectMgrPtr& sub_mgr,
	                                   const ee0::SelectionSet<ee0::GameObjWithPos>& selection)
	: m_cam(cam)
	, m_vp(vp)
	, m_sub_mgr(sub_mgr)
	, m_selection(selection)
{
	m_last_pos.MakeInvalid();
}

bool NodeTranslateState::OnMousePress(int x, int y)
{
	m_last_pos.Set(x, y);
	return false;
}

bool NodeTranslateState::OnMouseRelease(int x, int y)
{
	m_sub_mgr->NotifyObservers(ee0::MSG_UPDATE_COMPONENTS);
	return false;
}

bool NodeTranslateState::OnMouseDrag(int x, int y)
{
	if (m_selection.IsEmpty()) {
		return false;
	}

	Translate(m_last_pos, sm::ivec2(x, y));
	m_last_pos.Set(x, y);

	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

	return false;
}

void NodeTranslateState::Translate(const sm::ivec2& last, const sm::ivec2& curr)
{
	m_selection.Traverse([&](const ee0::GameObjWithPos& nwp)->bool
	{
#ifndef GAME_OBJ_ECS
		auto& node = nwp.GetNode();
		auto& ctrans = node->GetUniqueComp<n3::CompTransform>();

		auto& aabb = node->GetUniqueComp<n3::CompAABB>().GetAABB();
		sm::vec3 node_center = aabb.Cube().Center() + aabb.Position();
		node_center = ctrans.GetTransformMat() * node_center;

//		auto node_pos = ctrans.GetPosition();
		auto cam_pos = m_cam.GetPos();

		float dist_center = sm::dis_pos3_to_pos3(node_center, cam_pos);
//		float dist_pos = sm::dis_pos3_to_pos3(node_pos, cam_pos);

		sm::vec3 _last = m_vp.TransPos3ScreenToDir(
			sm::vec2(static_cast<float>(last.x), static_cast<float>(last.y)), m_cam);
		sm::vec3 _curr = m_vp.TransPos3ScreenToDir(
			sm::vec2(static_cast<float>(curr.x), static_cast<float>(curr.y)), m_cam);

		ctrans.Translate((_curr - _last) * /*dist_pos*/dist_center);
#endif // GAME_OBJ_ECS

		return true;
	});
}

}