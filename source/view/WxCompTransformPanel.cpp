#include "ee3/WxCompTransformPanel.h"

#include <node3/CompTransform.h>

#include <ee0/SubjectMgr.h>
#include <ee0/MessageID.h>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

namespace ee3
{

WxCompTransformPanel::WxCompTransformPanel(wxWindow* parent, 
	                                       n3::CompTransform& trans,
	                                       const ee0::SubjectMgrPtr& sub_mgr)
	: ee0::WxCompPanel(parent, "Transform")
	, m_ctrans(trans)
	, m_sub_mgr(sub_mgr)
{
	InitLayout();
	Expand();
}

void WxCompTransformPanel::RefreshNodeComp()
{
	auto& pos = m_ctrans.GetPosition();
	m_pos_x->SetValue(std::to_string(pos.x));
	m_pos_y->SetValue(std::to_string(pos.y));
	m_pos_z->SetValue(std::to_string(pos.z));

	float roll, pitch, yaw;
	sm::Quaternion::TransToEulerAngle(m_ctrans.GetAngle(), roll, pitch, yaw);
	m_angle_x->SetValue(std::to_string(pitch));
	m_angle_y->SetValue(std::to_string(yaw));
	m_angle_z->SetValue(std::to_string(roll));

	auto& scale = m_ctrans.GetScale();
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

		auto& pos = m_ctrans.GetPosition();
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

		float roll, pitch, yaw;
		sm::Quaternion::TransToEulerAngle(m_ctrans.GetAngle(), roll, pitch, yaw);
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

		auto& scale = m_ctrans.GetScale();
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
	// pos
	if (event.GetId() == m_pos_x->GetId()) 
	{
		double x;
		m_pos_x->GetValue().ToDouble(&x);
		auto& pos = m_ctrans.GetPosition();
		m_ctrans.SetPosition(sm::vec3(x, pos.y, pos.z));
	}
	else if (event.GetId() == m_pos_y->GetId()) 
	{
		double y;
		m_pos_y->GetValue().ToDouble(&y);
		auto& pos = m_ctrans.GetPosition();
		m_ctrans.SetPosition(sm::vec3(pos.x, y, pos.z));
	}
	else if (event.GetId() == m_pos_z->GetId()) 
	{
		double z;
		m_pos_z->GetValue().ToDouble(&z);
		auto& pos = m_ctrans.GetPosition();
		m_ctrans.SetPosition(sm::vec3(pos.x, pos.y, z));
	}
	// angle
	else if (event.GetId() == m_angle_x->GetId())
	{
		float roll, pitch, yaw;
		sm::Quaternion::TransToEulerAngle(m_ctrans.GetAngle(), roll, pitch, yaw);

		double x;
		m_angle_x->GetValue().ToDouble(&x);

		m_ctrans.SetAngle(sm::Quaternion::CreateFromEulerAngle(roll, x, yaw));
	}
	else if (event.GetId() == m_angle_y->GetId())
	{
		float roll, pitch, yaw;
		sm::Quaternion::TransToEulerAngle(m_ctrans.GetAngle(), roll, pitch, yaw);

		double y;
		m_angle_y->GetValue().ToDouble(&y);

		m_ctrans.SetAngle(sm::Quaternion::CreateFromEulerAngle(roll, pitch, y));
	}
	else if (event.GetId() == m_angle_z->GetId())
	{
		float roll, pitch, yaw;
		sm::Quaternion::TransToEulerAngle(m_ctrans.GetAngle(), roll, pitch, yaw);

		double z;
		m_angle_z->GetValue().ToDouble(&z);

		m_ctrans.SetAngle(sm::Quaternion::CreateFromEulerAngle(z, pitch, yaw));
	}
	// scale
	else if (event.GetId() == m_scale_x->GetId())
	{
		double x;
		m_scale_x->GetValue().ToDouble(&x);
		auto& scale = m_ctrans.GetScale();
		m_ctrans.SetScale(sm::vec3(x, scale.y, scale.z));
	}
	else if (event.GetId() == m_scale_y->GetId())
	{
		double y;
		m_scale_y->GetValue().ToDouble(&y);
		auto& scale = m_ctrans.GetScale();
		m_ctrans.SetScale(sm::vec3(scale.x, y, scale.z));
	}
	else if (event.GetId() == m_scale_z->GetId())
	{
		double z;
		m_scale_z->GetValue().ToDouble(&z);
		auto& scale = m_ctrans.GetScale();
		m_ctrans.SetScale(sm::vec3(scale.x, scale.y, z));
	}

	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
}

}