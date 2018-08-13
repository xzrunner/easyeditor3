#include "ee3/NodeRotateOP.h"
#include "ee3/RotateAxisState.h"

#include <ee0/WxStagePage.h>
#include <ee0/SubjectMgr.h>

#include <node0/SceneNode.h>
#include <node3/CompAABB.h>
#include <node3/CompTransform.h>
#include <painting3/PerspCam.h>

namespace ee3
{

NodeRotateOP::NodeRotateOP(const std::shared_ptr<pt0::Camera>& camera,
	                       ee0::WxStagePage& stage, const pt3::Viewport& vp)
	: ee0::EditOP(camera)
	, m_sub_mgr(stage.GetSubjectMgr())
{
	InitRotateState(stage, vp);
}

bool NodeRotateOP::OnMouseLeftDown(int x, int y)
{
	bool ret = false;
	if (m_rotate_state->OnMousePress(x, y)) {
		ret = true;
	} else {
		ret = EditOP::OnMouseLeftDown(x, y);
	}

	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

	return ret;
}

bool NodeRotateOP::OnMouseLeftUp(int x, int y)
{
	if (EditOP::OnMouseLeftUp(x, y)) {
		return true;
	}

	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

	return m_rotate_state->OnMouseRelease(x, y);
}

bool NodeRotateOP::OnMouseDrag(int x, int y)
{
	if (EditOP::OnMouseDrag(x, y)) {
		return true;
	}

	return m_rotate_state->OnMouseDrag(x, y);
}

bool NodeRotateOP::OnActive()
{
	if (EditOP::OnActive()) {
		return true;
	}

	return m_rotate_state->OnActive(true);
}

bool NodeRotateOP::OnDraw() const
{
	if (EditOP::OnDraw()) {
		return true;
	}

	return m_rotate_state->OnDraw();
}

void NodeRotateOP::InitRotateState(ee0::WxStagePage& stage,
	                               const pt3::Viewport& vp)
{
	RotateAxisState::Callback cb;
	cb.is_need_draw = [&]() {
		return !stage.GetSelection().IsEmpty();
	};
	cb.get_origin_transform = [&](sm::vec3& pos, sm::mat4& mat) {
		//sm::cube tot_aabb;
		//m_selection.Traverse([&](const ee0::GameObjWithPos& opw)->bool
		//{
		//	auto aabb = opw.GetNode()->GetUniqueComp<n3::CompAABB>().GetAABB();
		//	auto trans_aabb = aabb.Cube();
		//	trans_aabb.Translate(aabb.Position());
		//	tot_aabb.Combine(trans_aabb);
		//	return false;
		//});

		//m_center = tot_aabb.Center();

		//////////////////////////////////////////////////////////////////////////

		sm::vec3 center;
		int count = 0;
		stage.GetSelection().Traverse([&](const ee0::GameObjWithPos& opw)->bool
		{
			++count;
			auto aabb = opw.GetNode()->GetUniqueComp<n3::CompAABB>().GetAABB();
			auto pos = opw.GetNode()->GetUniqueComp<n3::CompTransform>().GetTransformMat() * aabb.Cube().Center();
			center += pos;
			return false;
		});
		center /= static_cast<float>(count);
		pos = center;
	};
	cb.rotate = [&](const sm::Quaternion& delta) {
		stage.GetSelection().Traverse([&](const ee0::GameObjWithPos& nwp)->bool
		{
#ifndef GAME_OBJ_ECS
			auto& ctrans = nwp.GetNode()->GetUniqueComp<n3::CompTransform>();
			ctrans.Rotate(-delta);
#endif // GAME_OBJ_ECS
			return true;
		});
	};

	assert(m_camera->TypeID() == pt0::GetCamTypeID<pt3::PerspCam>());
	auto p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);

	m_rotate_state = std::make_shared<RotateAxisState>(
		p_cam, vp, m_sub_mgr, cb, RotateAxisState::Config());
}

}