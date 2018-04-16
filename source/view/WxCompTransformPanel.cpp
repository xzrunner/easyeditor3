#include "ee3/WxCompTransformPanel.h"

#include <ee0/SubjectMgr.h>
#include <ee0/MessageID.h>

#ifndef GAME_OBJ_ECS
#include <node0/SceneNode.h>
#include <node3/CompTransform.h>
#else
#include <ecsx/World.h>
#include <entity3/CompTransform.h>
#include <entity3/SysTransform.h>
#endif // GAME_OBJ_ECS

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

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
#ifndef GAME_OBJ_ECS
	auto& pos = m_obj->GetUniqueComp<n3::CompTransform>().GetPosition();
#else
	auto pos = e3::SysTransform::GetPosition(m_world, m_obj);
#endif // GAME_OBJ_ECS
	m_pos_x->SetValue(std::to_string(pos.x));
	m_pos_y->SetValue(std::to_string(pos.y));
	m_pos_z->SetValue(std::to_string(pos.z));

#ifndef GAME_OBJ_ECS
	auto& angle = m_obj->GetUniqueComp<n3::CompTransform>().GetAngle();
#else
	auto angle = e3::SysTransform::GetAngle(m_world, m_obj);
#endif // GAME_OBJ_ECS
	float roll, pitch, yaw;
	sm::Quaternion::TransToEulerAngle(angle, roll, pitch, yaw);
	m_angle_x->SetValue(std::to_string(pitch));
	m_angle_y->SetValue(std::to_string(yaw));
	m_angle_z->SetValue(std::to_string(roll));

#ifndef GAME_OBJ_ECS
	auto& scale = m_obj->GetUniqueComp<n3::CompTransform>().GetScale();
#else
	auto scale = e3::SysTransform::GetScale(m_world, m_obj);
#endif // GAME_OBJ_ECS
	m_scale_x->SetValue(std::to_string(scale.x));
	m_scale_y->SetValue(std::to_string(scale.y));
	m_scale_z->SetValue(std::to_string(scale.z));
}

void WxCompTransformPanel::InitLayout()
{
	wxWindow* win = GetPane();

	wxSizer* pane_sizer = new wxBoxSizer(wxVERTICAL);

	static const wxSize INPUT_SIZE(65, 19);

	// position
	{
		wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);

		sizer->Add(new wxStaticText(win, wxID_ANY, wxT("Position  ")));

#ifndef GAME_OBJ_ECS
		auto& pos = m_obj->GetUniqueComp<n3::CompTransform>().GetPosition();
#else
		auto pos = e3::SysTransform::GetPosition(m_world, m_obj);
#endif // GAME_OBJ_ECS		
		sizer->Add(new wxStaticText(win, wxID_ANY, wxT("X ")));
		sizer->Add(m_pos_x = new wxTextCtrl(win, wxID_ANY, std::to_string(pos.x), 
			wxDefaultPosition, INPUT_SIZE, wxEVT_COMMAND_TEXT_ENTER));
		sizer->Add(new wxStaticText(win, wxID_ANY, wxT("  Y ")));
		sizer->Add(m_pos_y = new wxTextCtrl(win, wxID_ANY, std::to_string(pos.y), 
			wxDefaultPosition, INPUT_SIZE, wxEVT_COMMAND_TEXT_ENTER));
		sizer->Add(new wxStaticText(win, wxID_ANY, wxT("  Z ")));
		sizer->Add(m_pos_z = new wxTextCtrl(win, wxID_ANY, std::to_string(pos.z), 
			wxDefaultPosition, INPUT_SIZE, wxEVT_COMMAND_TEXT_ENTER));

		Connect(m_pos_x->GetId(), wxEVT_COMMAND_TEXT_ENTER, 
			wxCommandEventHandler(WxCompTransformPanel::EnterTextValue));
		Connect(m_pos_y->GetId(), wxEVT_COMMAND_TEXT_ENTER, 
			wxCommandEventHandler(WxCompTransformPanel::EnterTextValue));
		Connect(m_pos_z->GetId(), wxEVT_COMMAND_TEXT_ENTER, 
			wxCommandEventHandler(WxCompTransformPanel::EnterTextValue));

		pane_sizer->Add(sizer);
	}
	// angle
	{
		wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);

		sizer->Add(new wxStaticText(win, wxID_ANY, wxT("Angle     ")));

#ifndef GAME_OBJ_ECS
		auto& angle = m_obj->GetUniqueComp<n3::CompTransform>().GetAngle();
#else
		auto angle = e3::SysTransform::GetAngle(m_world, m_obj);
#endif // GAME_OBJ_ECS
		float roll, pitch, yaw;
		sm::Quaternion::TransToEulerAngle(angle, roll, pitch, yaw);
		sizer->Add(new wxStaticText(win, wxID_ANY, wxT("X ")));
		sizer->Add(m_angle_x = new wxTextCtrl(win, wxID_ANY, std::to_string(pitch), 
			wxDefaultPosition, INPUT_SIZE, wxTE_PROCESS_ENTER));
		sizer->Add(new wxStaticText(win, wxID_ANY, wxT("  Y ")));
		sizer->Add(m_angle_y = new wxTextCtrl(win, wxID_ANY, std::to_string(yaw), 
			wxDefaultPosition, INPUT_SIZE, wxTE_PROCESS_ENTER));
		sizer->Add(new wxStaticText(win, wxID_ANY, wxT("  Z ")));
		sizer->Add(m_angle_z = new wxTextCtrl(win, wxID_ANY, std::to_string(roll), 
			wxDefaultPosition, INPUT_SIZE, wxTE_PROCESS_ENTER));

		Connect(m_angle_x->GetId(), wxEVT_COMMAND_TEXT_ENTER, 
			wxCommandEventHandler(WxCompTransformPanel::EnterTextValue));
		Connect(m_angle_y->GetId(), wxEVT_COMMAND_TEXT_ENTER, 
			wxCommandEventHandler(WxCompTransformPanel::EnterTextValue));
		Connect(m_angle_z->GetId(), wxEVT_COMMAND_TEXT_ENTER, 
			wxCommandEventHandler(WxCompTransformPanel::EnterTextValue));

		pane_sizer->Add(sizer);
	}
	// scale
	{
		wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);

		sizer->Add(new wxStaticText(win, wxID_ANY, wxT("Scale     ")));

#ifndef GAME_OBJ_ECS
		auto& scale = m_obj->GetUniqueComp<n3::CompTransform>().GetScale();
#else
		auto scale = e3::SysTransform::GetScale(m_world, m_obj);
#endif // GAME_OBJ_ECS
		sizer->Add(new wxStaticText(win, wxID_ANY, wxT("X ")));
		sizer->Add(m_scale_x = new wxTextCtrl(win, wxID_ANY, std::to_string(scale.x), 
			wxDefaultPosition, INPUT_SIZE, wxTE_PROCESS_ENTER));
		sizer->Add(new wxStaticText(win, wxID_ANY, wxT("  Y ")));
		sizer->Add(m_scale_y = new wxTextCtrl(win, wxID_ANY, std::to_string(scale.y), 
			wxDefaultPosition, INPUT_SIZE, wxTE_PROCESS_ENTER));
		sizer->Add(new wxStaticText(win, wxID_ANY, wxT("  Z ")));
		sizer->Add(m_scale_z = new wxTextCtrl(win, wxID_ANY, std::to_string(scale.z), 
			wxDefaultPosition, INPUT_SIZE, wxTE_PROCESS_ENTER));

		Connect(m_scale_x->GetId(), wxEVT_COMMAND_TEXT_ENTER, 
			wxCommandEventHandler(WxCompTransformPanel::EnterTextValue));
		Connect(m_scale_y->GetId(), wxEVT_COMMAND_TEXT_ENTER, 
			wxCommandEventHandler(WxCompTransformPanel::EnterTextValue));
		Connect(m_scale_z->GetId(), wxEVT_COMMAND_TEXT_ENTER, 
			wxCommandEventHandler(WxCompTransformPanel::EnterTextValue));

		pane_sizer->Add(sizer);
	}

	win->SetSizer(pane_sizer);
	pane_sizer->SetSizeHints(win);
}

void WxCompTransformPanel::EnterTextValue(wxCommandEvent& event)
{
#ifndef GAME_OBJ_ECS
	auto& ctrans = m_obj->GetUniqueComp<n3::CompTransform>();
#endif // GAME_OBJ_ECS

	// pos
	if (event.GetId() == m_pos_x->GetId()) 
	{
		double x;
		m_pos_x->GetValue().ToDouble(&x);
#ifndef GAME_OBJ_ECS
		auto& pos = ctrans.GetPosition();
		ctrans.SetPosition(sm::vec3(x, pos.y, pos.z));
#else
		auto pos = e3::SysTransform::GetPosition(m_world, m_obj);
		e3::SysTransform::SetPosition(m_world, m_obj, sm::vec3(x, pos.y, pos.z));
#endif // GAME_OBJ_ECS
	}
	else if (event.GetId() == m_pos_y->GetId()) 
	{
		double y;
		m_pos_y->GetValue().ToDouble(&y);
#ifndef GAME_OBJ_ECS
		auto& pos = ctrans.GetPosition();
		ctrans.SetPosition(sm::vec3(pos.x, y, pos.z));
#else
		auto pos = e3::SysTransform::GetPosition(m_world, m_obj);
		e3::SysTransform::SetPosition(m_world, m_obj, sm::vec3(pos.x, y, pos.z));
#endif // GAME_OBJ_ECS
	}
	else if (event.GetId() == m_pos_z->GetId()) 
	{
		double z;
		m_pos_z->GetValue().ToDouble(&z);
#ifndef GAME_OBJ_ECS
		auto& pos = ctrans.GetPosition();
		ctrans.SetPosition(sm::vec3(pos.x, pos.y, z));
#else
		auto pos = e3::SysTransform::GetPosition(m_world, m_obj);
		e3::SysTransform::SetPosition(m_world, m_obj, sm::vec3(pos.x, pos.y, z));
#endif // GAME_OBJ_ECS
	}
	// angle
	else if (event.GetId() == m_angle_x->GetId())
	{
		double x;
		m_angle_x->GetValue().ToDouble(&x);

		float roll, pitch, yaw;
#ifndef GAME_OBJ_ECS
		sm::Quaternion::TransToEulerAngle(ctrans.GetAngle(), roll, pitch, yaw);
		ctrans.SetAngle(sm::Quaternion::CreateFromEulerAngle(roll, x, yaw));
#else
		auto angle = e3::SysTransform::GetAngle(m_world, m_obj);
		sm::Quaternion::TransToEulerAngle(angle, roll, pitch, yaw);
		e3::SysTransform::SetAngle(m_world, m_obj, 
			sm::Quaternion::CreateFromEulerAngle(roll, x, yaw));
#endif // GAME_OBJ_ECS
	}
	else if (event.GetId() == m_angle_y->GetId())
	{
		double y;
		m_angle_y->GetValue().ToDouble(&y);

		float roll, pitch, yaw;
#ifndef GAME_OBJ_ECS
		sm::Quaternion::TransToEulerAngle(ctrans.GetAngle(), roll, pitch, yaw);
		ctrans.SetAngle(sm::Quaternion::CreateFromEulerAngle(roll, pitch, y));
#else
		auto angle = e3::SysTransform::GetAngle(m_world, m_obj);
		sm::Quaternion::TransToEulerAngle(angle, roll, pitch, yaw);
		e3::SysTransform::SetAngle(m_world, m_obj, 
			sm::Quaternion::CreateFromEulerAngle(roll, pitch, y));
#endif // GAME_OBJ_ECS
	}
	else if (event.GetId() == m_angle_z->GetId())
	{
		double z;
		m_angle_z->GetValue().ToDouble(&z);

		float roll, pitch, yaw;
#ifndef GAME_OBJ_ECS
		sm::Quaternion::TransToEulerAngle(ctrans.GetAngle(), roll, pitch, yaw);
		ctrans.SetAngle(sm::Quaternion::CreateFromEulerAngle(z, pitch, yaw));
#else
		auto angle = e3::SysTransform::GetAngle(m_world, m_obj);
		sm::Quaternion::TransToEulerAngle(angle, roll, pitch, yaw);
		e3::SysTransform::SetAngle(m_world, m_obj,
			sm::Quaternion::CreateFromEulerAngle(z, pitch, yaw));
#endif // GAME_OBJ_ECS
	}
	// scale
	else if (event.GetId() == m_scale_x->GetId())
	{
		double x;
		m_scale_x->GetValue().ToDouble(&x);
#ifndef GAME_OBJ_ECS
		auto& scale = ctrans.GetScale();
		ctrans.SetScale(sm::vec3(x, scale.y, scale.z));
#else
		auto scale = e3::SysTransform::GetScale(m_world, m_obj);
		e3::SysTransform::SetScale(m_world, m_obj, sm::vec3(x, scale.y, scale.z));
#endif // GAME_OBJ_ECS
	}
	else if (event.GetId() == m_scale_y->GetId())
	{
		double y;
		m_scale_y->GetValue().ToDouble(&y);
#ifndef GAME_OBJ_ECS
		auto& scale = ctrans.GetScale();
		ctrans.SetScale(sm::vec3(scale.x, y, scale.z));
#else
		auto scale = e3::SysTransform::GetScale(m_world, m_obj);
		e3::SysTransform::SetScale(m_world, m_obj, sm::vec3(scale.x, y, scale.z));
#endif // GAME_OBJ_ECS
	}
	else if (event.GetId() == m_scale_z->GetId())
	{
		double z;
		m_scale_z->GetValue().ToDouble(&z);
#ifndef GAME_OBJ_ECS
		auto& scale = ctrans.GetScale();
		ctrans.SetScale(sm::vec3(scale.x, scale.y, z));
#else
		auto scale = e3::SysTransform::GetScale(m_world, m_obj);
		e3::SysTransform::SetScale(m_world, m_obj, sm::vec3(scale.x, scale.y, z));
#endif // GAME_OBJ_ECS
	}

	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
}

}