#include "ee3/WxCompLightPanel.h"

#include <ee0/ReflectPropTypes.h>

#include <node0/SceneNode.h>
#include <node3/CompLight.h>
#include <painting3/Light.h>
#include <painting3/PointLight.h>
#include <painting3/DirectionalLight.h>
#include <js/RTTR.h>

#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>

namespace ee3
{

WxCompLightPanel::WxCompLightPanel(wxWindow* parent, const ee0::SubjectMgrPtr& sub_mgr, const ee0::GameObj& obj)
    : ee0::WxCompPanel(parent, "Model")
    , m_sub_mgr(sub_mgr)
    , m_obj(obj)
{
    InitLayout();
    Expand();
}

void WxCompLightPanel::InitLayout()
{
    wxWindow* win = GetPane();
    wxSizer* pane_sizer = new wxBoxSizer(wxVERTICAL);

    auto& clight = m_obj->GetUniqueComp<n3::CompLight>();
    auto light = clight.GetLight();
    if (!light) {
        return;
    }

    auto type = light->get_type();

    pane_sizer->Add(new wxStaticText(win, wxID_ANY, type.get_name().to_string()));

    for (auto& prop : type.get_properties())
    {
        if (prop.get_metadata(js::RTTR::NoSerializeTag())) {
            continue;
        }

        auto ui_info_obj = prop.get_metadata(ee0::UIMetaInfoTag());
        if (ui_info_obj.is_valid())
        {
            auto ui_info = ui_info_obj.get_value<ee0::UIMetaInfo>();
            InitControl(pane_sizer, ui_info, *light, prop);
        }
        else
        {
            ee0::UIMetaInfo ui_info(prop.get_name().to_string());
            InitControl(pane_sizer, ui_info, *light, prop);
        }
    }

    win->SetSizer(pane_sizer);
    pane_sizer->SetSizeHints(win);
}

}