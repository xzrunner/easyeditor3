#pragma once

#include <ee0/typedef.h>

#include <SM_Vector.h>
#include <SM_Quaternion.h>

#include <wx/window.h>

class wxTextCtrl;

namespace ee3
{

class WxTransformCtrl : public wxWindow
{
public:
	struct Callback
	{
		std::function<const sm::vec3&()>       get_position;
		std::function<const sm::Quaternion&()> get_rotation;
		std::function<const sm::vec3&()>       get_scaling;

		std::function<void(const sm::vec3&)>       set_position;
		std::function<void(const sm::Quaternion&)> set_rotation;
		std::function<void(const sm::vec3&)>       set_scaling;
	};

public:
	WxTransformCtrl(wxWindow* parent, const ee0::SubjectMgrPtr& sub_mgr,
		const Callback& cb);

	void RefreshView();

private:
	void InitLayout();

	void EnterTextValue(wxCommandEvent& event);

private:
	ee0::SubjectMgrPtr m_sub_mgr;

	Callback m_cb;

	wxTextCtrl *m_pos_x, *m_pos_y, *m_pos_z;
	wxTextCtrl *m_angle_x, *m_angle_y, *m_angle_z;
	wxTextCtrl *m_scale_x, *m_scale_y, *m_scale_z;

}; // WxTransformCtrl

}