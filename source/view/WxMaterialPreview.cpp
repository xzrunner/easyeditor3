#include "ee3/WxMaterialPreview.h"

#include <ee0/RenderContext.h>
#include <ee3/WorldTravelOP.h>

#include <painting3/RenderSystem.h>
#include <painting3/PerspCam.h>
#include <painting3/Blackboard.h>
#include <painting3/WindowContext.h>

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
	m_canvas = std::make_unique<Canvas>(this, m_edit_impl, rc, m_sub_mgr, m_material, user_effect);

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

WxMaterialPreview::Canvas::Canvas(wxWindow* parent, ee0::EditPanelImpl& edit_impl,
	                              const ee0::RenderContext* rc, const ee0::SubjectMgrPtr& sub_mgr,
	                              const pt0::Material& material, bool user_effect)
	: ee0::WxStageCanvas(parent, edit_impl, std::make_shared<pt3::PerspCam>(sm::vec3(0, 0, -1.5f), sm::vec3(0, 0, 0), sm::vec3(0, 1, 0)), rc, nullptr, HAS_3D)
	, m_sub_mgr(sub_mgr)
	, m_material(material)
	, m_user_effect(user_effect)
{
	sub_mgr->RegisterObserver(ee0::MSG_SET_CANVAS_DIRTY, this);
}

WxMaterialPreview::Canvas::~Canvas()
{
	m_sub_mgr->UnregisterObserver(ee0::MSG_SET_CANVAS_DIRTY, this);
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
	auto& wc = pt3::Blackboard::Instance()->GetWindowContext();
	if (!wc) {
		return;
	}

	ee0::RenderContext::Reset3D(true);

	pt3::RenderParams params;
	params.user_effect = m_user_effect;

    pt3::RenderContext ctx;
    auto sz = GetSize();
    ctx.resolution.x = sz.x;
    ctx.resolution.y = sz.y;
    auto cam_type = m_camera->TypeID();
    if (cam_type == pt0::GetCamTypeID<pt3::PerspCam>())
    {
        auto& p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);
        ctx.cam_pos = p_cam->GetPos();
    }

	pt3::RenderSystem::Instance()->DrawMaterial(m_material, params, ctx);
}

}