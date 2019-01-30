#include "ee3/WxCompModelPanel.h"

#include <ee0/WxMaterialCtrl.h>

#include <node0/SceneNode.h>
#include <node3/CompModel.h>

#include <wx/sizer.h>
#include <wx/textctrl.h>

namespace ee3
{

WxCompModelPanel::WxCompModelPanel(wxWindow* parent, const ee0::SubjectMgrPtr& sub_mgr, const ee0::GameObj& obj)
    : ee0::WxCompPanel(parent, "Model")
    , m_sub_mgr(sub_mgr)
    , m_obj(obj)
{
    InitLayout();
    Expand();
}

void WxCompModelPanel::RefreshNodeComp()
{
    auto& cmodel = m_obj->GetSharedComp<n3::CompModel>();
    m_filepath->SetValue(cmodel.GetFilepath());
}

void WxCompModelPanel::InitLayout()
{
    wxWindow* win = GetPane();
    wxSizer* pane_sizer = new wxBoxSizer(wxVERTICAL);

    auto& cmodel = m_obj->GetSharedComp<n3::CompModel>();

    // filepath
	{
		wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
		sizer->Add(m_filepath = new wxTextCtrl(win, wxID_ANY, cmodel.GetFilepath(),
			wxDefaultPosition, wxSize(200, 20), wxTE_READONLY));
		pane_sizer->Add(sizer);
	}

    // materials
    auto& mats = cmodel.GetAllMaterials();
    for (auto& m : mats) {
        pane_sizer->Add(new ee0::WxMaterialCtrl(win, m_sub_mgr, const_cast<pt0::Material&>(m)));
    }

    win->SetSizer(pane_sizer);
    pane_sizer->SetSizeHints(win);
}

}