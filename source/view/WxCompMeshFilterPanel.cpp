#include "ee3/WxCompMeshFilterPanel.h"

#include <ee0/MessageID.h>
#include <ee0/SubjectMgr.h>

#include <node0/SceneNode.h>
#include <node3/CompMeshFilter.h>
#include <node3/CompAABB.h>
#include <model/ParametricEquations.h>

#include <wx/sizer.h>
#include <wx/choice.h>

namespace ee3
{

std::vector<WxCompMeshFilterPanel::MeshInfo> WxCompMeshFilterPanel::m_meshes_info =
{
    { "Cone",        model::Cone::TYPE_NAME },
    { "Sphere",      model::Sphere::TYPE_NAME },
    { "Torus",       model::Torus::TYPE_NAME },
    { "TrefoilKnot", model::TrefoilKnot::TYPE_NAME },
    { "MobiusStrip", model::MobiusStrip::TYPE_NAME },
    { "KleinBottle", model::KleinBottle::TYPE_NAME },
};

WxCompMeshFilterPanel::WxCompMeshFilterPanel(const ur2::Device& dev, wxWindow* parent,
                                             const ee0::SubjectMgrPtr& sub_mgr, const ee0::GameObj& obj)
    : ee0::WxCompPanel(parent, "MeshFilter")
    , m_sub_mgr(sub_mgr)
    , m_obj(obj)
    , m_dev(dev)
{
    InitLayout();
    Expand();
}

void WxCompMeshFilterPanel::RefreshNodeComp()
{
    SetControlFromObj();
}

void WxCompMeshFilterPanel::InitLayout()
{
    wxWindow* win = GetPane();
    wxSizer* pane_sizer = new wxBoxSizer(wxVERTICAL);

    auto& cmesh = m_obj->GetUniqueComp<n3::CompMeshFilter>();
    auto& mesh = cmesh.GetMesh();
    if (mesh)
    {
        wxArrayString mesh_types;
        for (auto& m : m_meshes_info) {
            mesh_types.Add(m.title);
        }
        m_mesh_type = new wxChoice(win, wxID_ANY, wxDefaultPosition, wxDefaultSize, mesh_types);
        Connect(m_mesh_type->GetId(), wxEVT_COMMAND_CHOICE_SELECTED,
            wxCommandEventHandler(WxCompMeshFilterPanel::OnMeshTypeChanged));
        SetControlFromObj();

        pane_sizer->Add(m_mesh_type);
    }

    win->SetSizer(pane_sizer);
    pane_sizer->SetSizeHints(win);
}

void WxCompMeshFilterPanel::OnMeshTypeChanged(wxCommandEvent& event)
{
    auto& cmesh = m_obj->GetUniqueComp<n3::CompMeshFilter>();
    cmesh.SetMesh(m_dev, m_meshes_info[m_mesh_type->GetSelection()].name);

    // update aabb
    auto& caabb = m_obj->GetUniqueComp<n3::CompAABB>();
    caabb.SetAABB(cmesh.GetAABB());

    m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
}

void WxCompMeshFilterPanel::SetControlFromObj()
{
    int idx = -1;
    auto& cmesh = m_obj->GetUniqueComp<n3::CompMeshFilter>();
    auto& mesh_name = cmesh.GetMeshName();
    for (int i = 0, n = m_meshes_info.size(); i < n; ++i) {
        if (m_meshes_info[i].name == mesh_name) {
            idx = i;
            break;
        }
    }
    m_mesh_type->SetSelection(idx);
}

}