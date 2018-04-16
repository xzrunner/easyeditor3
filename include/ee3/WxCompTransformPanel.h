#pragma once

#include <ee0/WxCompPanel.h>
#include <ee0/typedef.h>
#include <ee0/GameObj.h>

ECS_WORLD_DECL
class wxTextCtrl;

namespace ee3
{

class WxCompTransformPanel : public ee0::WxCompPanel
{
public:
	WxCompTransformPanel(wxWindow* parent, const ee0::SubjectMgrPtr& sub_mgr,
		ECS_WORLD_PARAM const ee0::GameObj& obj);

	virtual void RefreshNodeComp() override;

private:
	void InitLayout();

	void EnterTextValue(wxCommandEvent& event);

private:
	ee0::SubjectMgrPtr m_sub_mgr;
	ECS_WORLD_SELF_DEF
	ee0::GameObj       m_obj;

	wxTextCtrl *m_pos_x, *m_pos_y, *m_pos_z;
	wxTextCtrl *m_angle_x, *m_angle_y, *m_angle_z;
	wxTextCtrl *m_scale_x, *m_scale_y, *m_scale_z;

}; // WxCompTransformPanel

}