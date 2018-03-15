#pragma once

#include <ee0/WxCompPanel.h>
#include <ee0/typedef.h>

#include <node3/CompTransform.h>

class wxTextCtrl;

namespace ee3
{

class WxCompTransformPanel : public ee0::WxCompPanel
{
public:
	WxCompTransformPanel(wxWindow* parent, n3::CompTransform& trans,
		const ee0::SubjectMgrPtr& sub_mgr);

	virtual void RefreshNodeComp() override;

private:
	void InitLayout();

	void EnterTextValue(wxCommandEvent& event);

private:
	n3::CompTransform& m_ctrans;
	ee0::SubjectMgrPtr m_sub_mgr;

	wxTextCtrl *m_pos_x, *m_pos_y, *m_pos_z;
	wxTextCtrl *m_angle_x, *m_angle_y, *m_angle_z;
	wxTextCtrl *m_scale_x, *m_scale_y, *m_scale_z;

}; // WxCompTransformPanel

}