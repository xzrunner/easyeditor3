#include "ee3/WxTransformCtrl.h"

#include <ee0/SubjectMgr.h>
#include <ee0/MessageID.h>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

namespace ee3
{

WxTransformCtrl::WxTransformCtrl(wxWindow* parent, const ee0::SubjectMgrPtr& sub_mgr, const Callback& cb)
	: wxWindow(parent, wxID_ANY)
	, m_sub_mgr(sub_mgr)
	, m_cb(cb)
{
	InitLayout();
}

void WxTransformCtrl::RefreshView()
{
	auto& pos = m_cb.get_position();
	m_pos_x->SetValue(std::to_string(pos.x));
	m_pos_y->SetValue(std::to_string(pos.y));
	m_pos_z->SetValue(std::to_string(pos.z));

	auto& angle = m_cb.get_rotation();
	float roll, pitch, yaw;
	sm::Quaternion::TransToEulerAngle(angle, roll, pitch, yaw);
	m_angle_x->SetValue(std::to_string(pitch));
	m_angle_y->SetValue(std::to_string(yaw));
	m_angle_z->SetValue(std::to_string(roll));

	auto& scale = m_cb.get_scaling();
	m_scale_x->SetValue(std::to_string(scale.x));
	m_scale_y->SetValue(std::to_string(scale.y));
	m_scale_z->SetValue(std::to_string(scale.z));
}

void WxTransformCtrl::InitLayout()
{
	wxSizer* pane_sizer = new wxBoxSizer(wxVERTICAL);

	static const wxSize INPUT_SIZE(65, 19);

	// position
	{
		wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);

		sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Position  ")));

		auto& position = m_cb.get_position();
		sizer->Add(new wxStaticText(this, wxID_ANY, wxT("X ")));
		sizer->Add(m_pos_x = new wxTextCtrl(this, wxID_ANY, std::to_string(position.x),
			wxDefaultPosition, INPUT_SIZE, wxEVT_COMMAND_TEXT_ENTER));
		sizer->Add(new wxStaticText(this, wxID_ANY, wxT("  Y ")));
		sizer->Add(m_pos_y = new wxTextCtrl(this, wxID_ANY, std::to_string(position.y),
			wxDefaultPosition, INPUT_SIZE, wxEVT_COMMAND_TEXT_ENTER));
		sizer->Add(new wxStaticText(this, wxID_ANY, wxT("  Z ")));
		sizer->Add(m_pos_z = new wxTextCtrl(this, wxID_ANY, std::to_string(position.z),
			wxDefaultPosition, INPUT_SIZE, wxEVT_COMMAND_TEXT_ENTER));

		Connect(m_pos_x->GetId(), wxEVT_COMMAND_TEXT_ENTER,
			wxCommandEventHandler(WxTransformCtrl::EnterTextValue));
		Connect(m_pos_y->GetId(), wxEVT_COMMAND_TEXT_ENTER,
			wxCommandEventHandler(WxTransformCtrl::EnterTextValue));
		Connect(m_pos_z->GetId(), wxEVT_COMMAND_TEXT_ENTER,
			wxCommandEventHandler(WxTransformCtrl::EnterTextValue));

		pane_sizer->Add(sizer);
	}
	// angle
	{
		wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);

		sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Angle     ")));

		float roll, pitch, yaw;
		sm::Quaternion::TransToEulerAngle(m_cb.get_rotation(), roll, pitch, yaw);
		sizer->Add(new wxStaticText(this, wxID_ANY, wxT("X ")));
		sizer->Add(m_angle_x = new wxTextCtrl(this, wxID_ANY, std::to_string(pitch),
			wxDefaultPosition, INPUT_SIZE, wxTE_PROCESS_ENTER));
		sizer->Add(new wxStaticText(this, wxID_ANY, wxT("  Y ")));
		sizer->Add(m_angle_y = new wxTextCtrl(this, wxID_ANY, std::to_string(yaw),
			wxDefaultPosition, INPUT_SIZE, wxTE_PROCESS_ENTER));
		sizer->Add(new wxStaticText(this, wxID_ANY, wxT("  Z ")));
		sizer->Add(m_angle_z = new wxTextCtrl(this, wxID_ANY, std::to_string(roll),
			wxDefaultPosition, INPUT_SIZE, wxTE_PROCESS_ENTER));

		Connect(m_angle_x->GetId(), wxEVT_COMMAND_TEXT_ENTER,
			wxCommandEventHandler(WxTransformCtrl::EnterTextValue));
		Connect(m_angle_y->GetId(), wxEVT_COMMAND_TEXT_ENTER,
			wxCommandEventHandler(WxTransformCtrl::EnterTextValue));
		Connect(m_angle_z->GetId(), wxEVT_COMMAND_TEXT_ENTER,
			wxCommandEventHandler(WxTransformCtrl::EnterTextValue));

		pane_sizer->Add(sizer);
	}
	// scale
	{
		wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);

		sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Scale     ")));

		auto& scaling = m_cb.get_scaling();
		sizer->Add(new wxStaticText(this, wxID_ANY, wxT("X ")));
		sizer->Add(m_scale_x = new wxTextCtrl(this, wxID_ANY, std::to_string(scaling.x),
			wxDefaultPosition, INPUT_SIZE, wxTE_PROCESS_ENTER));
		sizer->Add(new wxStaticText(this, wxID_ANY, wxT("  Y ")));
		sizer->Add(m_scale_y = new wxTextCtrl(this, wxID_ANY, std::to_string(scaling.y),
			wxDefaultPosition, INPUT_SIZE, wxTE_PROCESS_ENTER));
		sizer->Add(new wxStaticText(this, wxID_ANY, wxT("  Z ")));
		sizer->Add(m_scale_z = new wxTextCtrl(this, wxID_ANY, std::to_string(scaling.z),
			wxDefaultPosition, INPUT_SIZE, wxTE_PROCESS_ENTER));

		Connect(m_scale_x->GetId(), wxEVT_COMMAND_TEXT_ENTER,
			wxCommandEventHandler(WxTransformCtrl::EnterTextValue));
		Connect(m_scale_y->GetId(), wxEVT_COMMAND_TEXT_ENTER,
			wxCommandEventHandler(WxTransformCtrl::EnterTextValue));
		Connect(m_scale_z->GetId(), wxEVT_COMMAND_TEXT_ENTER,
			wxCommandEventHandler(WxTransformCtrl::EnterTextValue));

		pane_sizer->Add(sizer);
	}

	SetSizer(pane_sizer);
	pane_sizer->SetSizeHints(this);
}

void WxTransformCtrl::EnterTextValue(wxCommandEvent& event)
{
	// pos
	if (event.GetId() == m_pos_x->GetId())
	{
		double x;
		m_pos_x->GetValue().ToDouble(&x);
		auto& pos = m_cb.get_position();
		m_cb.set_position(sm::vec3(x, pos.y, pos.z));
	}
	else if (event.GetId() == m_pos_y->GetId())
	{
		double y;
		m_pos_y->GetValue().ToDouble(&y);
		auto& pos = m_cb.get_position();
		m_cb.set_position(sm::vec3(pos.x, y, pos.z));
	}
	else if (event.GetId() == m_pos_z->GetId())
	{
		double z;
		m_pos_z->GetValue().ToDouble(&z);
		auto& pos = m_cb.get_position();
		m_cb.set_position(sm::vec3(pos.x, pos.y, z));
	}
	// angle
	else if (event.GetId() == m_angle_x->GetId())
	{
		double x;
		m_angle_x->GetValue().ToDouble(&x);

		float roll, pitch, yaw;
		sm::Quaternion::TransToEulerAngle(m_cb.get_rotation(), roll, pitch, yaw);
		m_cb.set_rotation(sm::Quaternion::CreateFromEulerAngle(roll, x, yaw));
	}
	else if (event.GetId() == m_angle_y->GetId())
	{
		double y;
		m_angle_y->GetValue().ToDouble(&y);

		float roll, pitch, yaw;
		sm::Quaternion::TransToEulerAngle(m_cb.get_rotation(), roll, pitch, yaw);
		m_cb.set_rotation(sm::Quaternion::CreateFromEulerAngle(roll, pitch, y));
	}
	else if (event.GetId() == m_angle_z->GetId())
	{
		double z;
		m_angle_z->GetValue().ToDouble(&z);

		float roll, pitch, yaw;
		sm::Quaternion::TransToEulerAngle(m_cb.get_rotation(), roll, pitch, yaw);
		m_cb.set_rotation(sm::Quaternion::CreateFromEulerAngle(z, pitch, yaw));
	}
	// scale
	else if (event.GetId() == m_scale_x->GetId())
	{
		double x;
		m_scale_x->GetValue().ToDouble(&x);
		auto& scale = m_cb.get_scaling();
		m_cb.set_scaling(sm::vec3(x, scale.y, scale.z));
	}
	else if (event.GetId() == m_scale_y->GetId())
	{
		double y;
		m_scale_y->GetValue().ToDouble(&y);
		auto& scale = m_cb.get_scaling();
		m_cb.set_scaling(sm::vec3(scale.x, y, scale.z));
	}
	else if (event.GetId() == m_scale_z->GetId())
	{
		double z;
		m_scale_z->GetValue().ToDouble(&z);
		auto& scale = m_cb.get_scaling();
		m_cb.set_scaling(sm::vec3(scale.x, scale.y, z));
	}

	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
}

}