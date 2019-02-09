#pragma once

#include <ee0/WxCompPanel.h>
#include <ee0/typedef.h>
#include <ee0/GameObj.h>

class wxChoice;

namespace ee3
{

class WxCompMeshFilterPanel : public ee0::WxCompPanel
{
public:
    WxCompMeshFilterPanel(wxWindow* parent, const ee0::SubjectMgrPtr& sub_mgr,
        const ee0::GameObj& obj);

    virtual void RefreshNodeComp() override;

private:
    void InitLayout();

    void OnMeshTypeChanged(wxCommandEvent& event);

    void SetControlFromObj();

private:
    struct MeshInfo
    {
        std::string title;
        std::string name;
    };

private:
    ee0::SubjectMgrPtr m_sub_mgr;
    ee0::GameObj       m_obj;

    wxChoice* m_mesh_type;

    // config
    static std::vector<MeshInfo> m_meshes_info;

}; // WxCompMeshFilterPanel

}