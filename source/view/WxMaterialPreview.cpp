#include "ee3/WxMaterialPreview.h"

#include <ee0/RenderContext.h>
#include <ee3/WorldTravelOP.h>

#include <painting3/RenderSystem.h>
#include <painting3/PerspCam.h>
#include <painting3/Blackboard.h>
#include <painting3/WindowContext.h>
#include <painting3/MaterialMgr.h>
#include <shaderweaver/node/Raymarching.h>
#include <shaderweaver/node/CameraPos.h>
#include <facade/ImageCube.h>

#include <wx/sizer.h>

namespace ee3
{

WxMaterialPreview::WxMaterialPreview(wxWindow* parent, const sm::ivec2& size,
	                                 const ee0::SubjectMgrPtr& sub_mgr,
	                                 const ee0::RenderContext* rc, bool user_effect)
	: wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(size.x, size.y))
	, m_sub_mgr(sub_mgr)
	, m_edit_impl(this, m_sub_mgr)
{
	m_canvas = std::make_unique<Canvas>(this, rc, user_effect);

	auto op = std::make_shared<ee3::WorldTravelOP>(
		m_canvas->GetCamera(), m_canvas->GetViewport(), m_sub_mgr
	);
	m_edit_impl.SetEditOP(op);

	Bind(wxEVT_SIZE, &WxMaterialPreview::OnSize, this, GetId());
}

void WxMaterialPreview::RefreshCanvas()
{
	m_canvas->SetDirty();
}

void WxMaterialPreview::OnSize(wxSizeEvent& event)
{
	m_canvas->SetSize(event.GetSize());
}

//////////////////////////////////////////////////////////////////////////
// class WxMaterialPreview::Canvas
//////////////////////////////////////////////////////////////////////////

WxMaterialPreview::Canvas::Canvas(WxMaterialPreview* panel, const ee0::RenderContext* rc, bool user_effect)
	: ee0::WxStageCanvas(panel, panel->m_edit_impl, std::make_shared<pt3::PerspCam>(sm::vec3(0, 0, -1.5f), sm::vec3(0, 0, 0), sm::vec3(0, 1, 0)), rc, nullptr, HAS_3D)
    , m_panel(panel)
	, m_user_effect(user_effect)
{
    panel->m_sub_mgr->RegisterObserver(ee0::MSG_SET_CANVAS_DIRTY, this);
}

WxMaterialPreview::Canvas::~Canvas()
{
    m_panel->m_sub_mgr->UnregisterObserver(ee0::MSG_SET_CANVAS_DIRTY, this);
}

void WxMaterialPreview::Canvas::OnNotify(uint32_t msg, const ee0::VariantSet& variants)
{
	switch (msg)
	{
	case ee0::MSG_SET_CANVAS_DIRTY:
		SetDirty();
		break;
	}
}

void WxMaterialPreview::Canvas::OnSize(int w, int h)
{
	auto& wc = pt3::Blackboard::Instance()->GetWindowContext();
	if (wc)
	{
		wc->SetScreen(w, h);
		m_viewport.SetSize(w, h);

		m_camera->OnSize(static_cast<float>(w), static_cast<float>(h));
		wc->SetProjection(m_camera->GetProjectionMat());
	}
}

void WxMaterialPreview::Canvas::OnDrawSprites() const
{
    ee0::RenderContext::Reset3D(true);

    DrawSkybox();
    DrawMaterial();
}

void WxMaterialPreview::Canvas::DrawSkybox() const
{
    if (m_panel->m_skybox) {
        auto tex = m_panel->m_skybox->GetTexture();
        if (tex) {
            pt3::RenderSystem::DrawSkybox(*tex);
        }
    }
}

void WxMaterialPreview::Canvas::DrawMaterial() const
{
	auto& wc = pt3::Blackboard::Instance()->GetWindowContext();
	if (!wc) {
		return;
	}

	pt3::RenderParams params;
	params.user_effect = m_user_effect;

    pt0::RenderContext ctx;
    auto sz = GetSize();
    ctx.AddVar(
        // fixme: get uniform name from shader
        sw::node::Raymarching::ResolutionName(),
        pt0::RenderVariant(sm::vec2(sz.x, sz.y))
    );
    if (m_camera->TypeID() == pt0::GetCamTypeID<pt3::PerspCam>())
    {
        auto& p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);
        ctx.AddVar(
            // fixme: get uniform name from shader
            sw::node::CameraPos::CamPosName(),
            pt0::RenderVariant(p_cam->GetPos())
        );
    }
    ctx.AddVar(
        pt3::MaterialMgr::PositionUniforms::light_pos.name,
        pt0::RenderVariant(sm::vec3(0, 2, -4))
    );
    ctx.AddVar(
        pt3::MaterialMgr::PosTransUniforms::view.name,
        pt0::RenderVariant(wc->GetViewMat())
    );
    ctx.AddVar(
        pt3::MaterialMgr::PosTransUniforms::projection.name,
        pt0::RenderVariant(wc->GetProjMat())
    );

    sm::mat4 model_mat;
    ctx.AddVar(
        pt3::MaterialMgr::PosTransUniforms::model.name,
        pt0::RenderVariant(model_mat)
    );
    auto normal_mat = model_mat.Inverted().Transposed();
    ctx.AddVar(
        pt3::MaterialMgr::PositionUniforms::normal_mat.name,
        pt0::RenderVariant(sm::mat3(normal_mat))
    );

	pt3::RenderSystem::Instance()->DrawMaterial(m_panel->m_material, params, ctx, m_shader);
}

}