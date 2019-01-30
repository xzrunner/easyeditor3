#pragma once

#include <ee0/WxCompPanel.h>
#include <ee0/typedef.h>
#include <ee0/GameObj.h>

class wxTextCtrl;

namespace ee3
{

class WxCompModelPanel : public ee0::WxCompPanel
{
public:
    WxCompModelPanel(wxWindow* parent, const ee0::SubjectMgrPtr& sub_mgr,
        const ee0::GameObj& obj);

    virtual void RefreshNodeComp() override;

private:
    void InitLayout();

private:
    ee0::SubjectMgrPtr m_sub_mgr;
    ee0::GameObj       m_obj;

    wxTextCtrl* m_filepath;

}; // WxCompModelPanel

}