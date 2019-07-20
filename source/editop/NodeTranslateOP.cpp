#include "ee3/NodeTranslateOP.h"
#include "ee3/TranslateAxisState.h"

#include <ee0/WxStagePage.h>
#include <ee0/SubjectMgr.h>

#include <node0/SceneNode.h>
#include <node3/CompAABB.h>
#include <node3/CompTransform.h>
#include <painting3/PerspCam.h>

namespace ee3
{

NodeTranslateOP::NodeTranslateOP(const std::shared_ptr<pt0::Camera>& camera,
	                             ee0::WxStagePage& stage, const pt3::Viewport& vp)
	: ee0::EditOP(camera)
	, m_sub_mgr(stage.GetSubjectMgr())
{
	InitTranslateState(stage, vp);
}

bool NodeTranslateOP::OnMouseLeftDown(int x, int y)
{
	bool ret = false;
	if (m_translate_state->OnMousePress(x, y)) {
		ret = true;
	} else {
		ret = EditOP::OnMouseLeftDown(x, y);
	}

	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

	return ret;
}

bool NodeTranslateOP::OnMouseLeftUp(int x, int y)
{
	if (EditOP::OnMouseLeftUp(x, y)) {
		return true;
	}

	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

	return m_translate_state->OnMouseRelease(x, y);
}

bool NodeTranslateOP::OnMouseDrag(int x, int y)
{
	if (EditOP::OnMouseDrag(x, y)) {
		return true;
	}

	return m_translate_state->OnMouseDrag(x, y);
}

bool NodeTranslateOP::OnActive()
{
	if (EditOP::OnActive()) {
		return true;
	}

	return m_translate_state->OnActive(true);
}

bool NodeTranslateOP::OnDraw() const
{
	if (EditOP::OnDraw()) {
		return true;
	}

	return m_translate_state->OnDraw();
}

void NodeTranslateOP::InitTranslateState(ee0::WxStagePage& stage, const pt3::Viewport& vp)
{
	TranslateAxisState::Callback cb;
	cb.is_need_draw = [&]() {
		return !stage.GetSelection().IsEmpty();
	};
	cb.get_origin_wmat = [&]()->sm::mat4 {
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
		sm::Quaternion angle;
		int count = 0;
		stage.GetSelection().Traverse([&](const ee0::GameObjWithPos& opw)->bool
		{
			++count;
			auto node = opw.GetNode();
			auto aabb = opw.GetNode()->GetUniqueComp<n3::CompAABB>().GetAABB();
			auto& ctrans = opw.GetNode()->GetUniqueComp<n3::CompTransform>();
			angle = ctrans.GetAngle();
			auto pos = ctrans.GetTransformMat() * aabb.Cube().Center();
			center += pos;
			return false;
		});
		center /= static_cast<float>(count);
		auto trans_mat = sm::mat4::Translated(center.x, center.y, center.z);
		auto rot_mat = sm::mat4(angle);
		return rot_mat * trans_mat;
	};
	cb.translate = [&](const sm::vec3& offset) {
		stage.GetSelection().Traverse([&](const ee0::GameObjWithPos& nwp)->bool
		{
#ifndef GAME_OBJ_ECS
			auto& ctrans = nwp.GetNode()->GetUniqueComp<n3::CompTransform>();
			ctrans.Translate(offset);
#endif // GAME_OBJ_ECS
			return true;
		});
	};

	assert(m_camera->TypeID() == pt0::GetCamTypeID<pt3::PerspCam>());
	auto p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);

	m_translate_state = std::make_shared<TranslateAxisState>(
		p_cam, vp, m_sub_mgr, cb, TranslateAxisState::Config());
}

}