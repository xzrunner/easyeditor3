#include "ee3/NodeTranslateState.h"

#include <ee0/SubjectMgr.h>
#include <ee0/MessageID.h>

#include <painting3/Camera.h>
#include <painting3/Viewport.h>
#ifndef GAME_OBJ_ECS
#include <node0/SceneNode.h>
#include <node3/CompTransform.h>
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

void NodeTranslateState::Translate(const sm::ivec2& first, const sm::ivec2& curr)
{
	m_selection.Traverse([&](const ee0::GameObjWithPos& nwp)->bool
	{
#ifndef GAME_OBJ_ECS
		auto& ctrans = nwp.GetNode()->GetUniqueComp<n3::CompTransform>();

		float dist = m_cam.GetToward().Dot(ctrans.GetPosition() - m_cam.GetPos());

		sm::vec3 _first = m_vp.TransPos3ScreenToDir(
			sm::vec2(static_cast<float>(first.x), static_cast<float>(first.y)), m_cam).Normalized() * dist;
		sm::vec3 _curr = m_vp.TransPos3ScreenToDir(
			sm::vec2(static_cast<float>(curr.x), static_cast<float>(curr.y)), m_cam).Normalized() * dist;
		ctrans.Translate(_curr - _first);
#endif // GAME_OBJ_ECS

		return true;
	});
}

}