#pragma once

#include <ee0/WxCompPanel.h>
#include <ee0/typedef.h>
#include <ee0/GameObj.h>

namespace ee3
{

class WxCompLightPanel : public ee0::WxCompPanel
{
public:
    WxCompLightPanel(wxWindow* parent, const ee0::SubjectMgrPtr& sub_mgr,
        const ee0::GameObj& obj);

private:
    void InitLayout();

private:
    ee0::SubjectMgrPtr m_sub_mgr;
    ee0::GameObj       m_obj;

}; // WxCompLightPanel

}