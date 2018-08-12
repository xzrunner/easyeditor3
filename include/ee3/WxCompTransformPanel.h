#pragma once

#include <ee0/WxCompPanel.h>
#include <ee0/typedef.h>
#include <ee0/GameObj.h>

ECS_WORLD_DECL

namespace ee3
{

class WxTransformCtrl;

class WxCompTransformPanel : public ee0::WxCompPanel
{
public:
	WxCompTransformPanel(wxWindow* parent, const ee0::SubjectMgrPtr& sub_mgr,
		ECS_WORLD_PARAM const ee0::GameObj& obj);

	virtual void RefreshNodeComp() override;

private:
	void InitLayout();

private:
	ee0::SubjectMgrPtr m_sub_mgr;
	ECS_WORLD_SELF_DEF
	ee0::GameObj       m_obj;

	WxTransformCtrl* m_trans_ctrl;

}; // WxCompTransformPanel

}