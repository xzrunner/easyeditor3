#include "ee3/WxCompTransformPanel.h"
#include "ee3/WxTransformCtrl.h"

#include <ee0/SubjectMgr.h>
#include <ee0/MessageID.h>

#ifndef GAME_OBJ_ECS
#include <node0/SceneNode.h>
#include <node3/CompTransform.h>
#else
#include <entity0/World.h>
#include <entity3/CompTransform.h>
#include <entity3/SysTransform.h>
#endif // GAME_OBJ_ECS

#include <wx/sizer.h>

namespace ee3
{

WxCompTransformPanel::WxCompTransformPanel(wxWindow* parent,
	                                       const ee0::SubjectMgrPtr& sub_mgr,
	                                       ECS_WORLD_PARAM
	                                       const ee0::GameObj& obj)
	: ee0::WxCompPanel(parent, "Transform")
	, m_sub_mgr(sub_mgr)
	ECS_WORLD_SELF_ASSIGN
	, m_obj(obj)
{
	InitLayout();
	Expand();
}

void WxCompTransformPanel::RefreshNodeComp()
{
	m_trans_ctrl->RefreshView();
}

void WxCompTransformPanel::InitLayout()
{
	WxTransformCtrl::Callback cb;
	cb.get_position = [&]()->const sm::vec3& {
#ifndef GAME_OBJ_ECS
		return m_obj->GetUniqueComp<n3::CompTransform>().GetPosition();
#else
		return e3::SysTransform::GetPosition(m_world, m_obj);
#endif // GAME_OBJ_ECS
	};
	cb.get_rotation = [&]()->const sm::Quaternion& {
#ifndef GAME_OBJ_ECS
		return m_obj->GetUniqueComp<n3::CompTransform>().GetAngle();
#else
		return e3::SysTransform::GetAngle(m_world, m_obj);
#endif // GAME_OBJ_ECS
	};
	cb.get_scaling = [&]()->const sm::vec3& {
#ifndef GAME_OBJ_ECS
		return m_obj->GetUniqueComp<n3::CompTransform>().GetScale();
#else
		return e3::SysTransform::GetScale(m_world, m_obj);
#endif // GAME_OBJ_ECS
	};
	cb.set_position = [&](const sm::vec3& pos) {
#ifndef GAME_OBJ_ECS
		auto& ctrans = m_obj->GetUniqueComp<n3::CompTransform>();
		ctrans.SetPosition(pos);
#else
		e3::SysTransform::SetPosition(m_world, m_obj, pos);
#endif // GAME_OBJ_ECS
	};
	cb.set_rotation = [&](const sm::Quaternion& rot) {
#ifndef GAME_OBJ_ECS
		auto& ctrans = m_obj->GetUniqueComp<n3::CompTransform>();
		ctrans.SetAngle(rot);
#else
		e3::SysTransform::SetAngle(m_world, m_obj, rot);
#endif // GAME_OBJ_ECS
	};
	cb.set_scaling = [&](const sm::vec3& scale) {
#ifndef GAME_OBJ_ECS
		auto& ctrans = m_obj->GetUniqueComp<n3::CompTransform>();
		ctrans.SetScale(scale);
#else
		e3::SysTransform::SetScale(m_world, m_obj, scale);
#endif // GAME_OBJ_ECS
	};

	wxWindow* win = GetPane();
	wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(m_trans_ctrl = new WxTransformCtrl(win, m_sub_mgr, cb));
	win->SetSizer(sizer);
	sizer->SetSizeHints(win);
}

}