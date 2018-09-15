#include "ee3/NodeRotateState.h"

#include <ee0/SubjectMgr.h>
#include <ee0/MessageID.h>

#include <painting3/PerspCam.h>
#include <painting3/Viewport.h>
#ifndef GAME_OBJ_ECS
#include <node0/SceneNode.h>
#include <node3/CompTransform.h>
#endif // GAME_OBJ_ECS

namespace ee3
{

NodeRotateState::NodeRotateState(const std::shared_ptr<pt0::Camera>& camera, 
	                             const pt3::Viewport& vp,
	                             const ee0::SubjectMgrPtr& sub_mgr,
	                             const ee0::SelectionSet<ee0::GameObjWithPos>& selection)
	: ee0::EditOpState(camera)
	, m_vp(vp)
	, m_sub_mgr(sub_mgr)
	, m_selection(selection)
{
	m_last_pos.MakeInvalid();
}

bool NodeRotateState::OnMousePress(int x, int y)
{
	m_last_pos.Set(x, y);

	return false;
}

bool NodeRotateState::OnMouseRelease(int x, int y)
{
	m_sub_mgr->NotifyObservers(ee0::MSG_UPDATE_COMPONENTS);

	return false;
}

bool NodeRotateState::OnMouseDrag(int x, int y)
{
	Rotate(m_last_pos, sm::ivec2(x, y));
	m_last_pos.Set(x, y);

	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

	return false;
}

void NodeRotateState::Rotate(const sm::ivec2& start, const sm::ivec2& end)
{
	if (m_camera->TypeID() == pt0::GetCamTypeID<pt3::PerspCam>())
	{
		auto p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);

		m_selection.Traverse([&](const ee0::GameObjWithPos& nwp)->bool
		{
	#ifndef GAME_OBJ_ECS
			auto& ctrans = nwp.GetNode()->GetUniqueComp<n3::CompTransform>();

			sm::vec2 center = TransPos3ProjectToScreen(ctrans.GetPosition());
			sm::vec2 base = TransPos3ProjectToScreen(sm::vec3(0, 0, 0));

   			sm::vec3 start3 = m_vp.MapToSphere(
				base + sm::vec2(static_cast<float>(start.x), static_cast<float>(start.y)) -  center);
   			sm::vec3 end3   = m_vp.MapToSphere(
				base + sm::vec2(static_cast<float>(end.x), static_cast<float>(end.y)) - center);

			auto cam_mat = p_cam->GetRotateMat().Inverted();
			start3 = cam_mat * start3;
			end3   = cam_mat * end3;
		
   			sm::Quaternion delta = sm::Quaternion::CreateFromVectors(start3, end3);
			ctrans.Rotate(-delta);
	#endif // GAME_OBJ_ECS

			return true;
		});
	}
}

sm::vec2 NodeRotateState::TransPos3ProjectToScreen(const sm::vec3& proj) const
{
	if (m_camera->TypeID() == pt0::GetCamTypeID<pt3::PerspCam>())
	{
		auto p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);

		// todo proj mat
		return m_vp.TransPos3ProjectToScreen(/*m_mat_projection * */m_camera->GetViewMat() * proj, *p_cam);
	}
	else
	{
		return sm::vec2();
	}
}

}