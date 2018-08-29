#include "ee3/WxMaterialPreview.h"

#include <ee0/RenderContext.h>

#include <unirender/RenderContext.h>
#include <painting3/Material.h>
#include <painting3/RenderSystem.h>
#include <painting3/PerspCam.h>
#include <painting3/Blackboard.h>
#include <painting3/WindowContext.h>
#include <facade/RenderContext.h>

#include <painting3/PrimitiveDraw.h>

#include <wx/sizer.h>

namespace ee3
{

WxMaterialPreview::WxMaterialPreview(wxWindow* parent, const sm::ivec2& size)
	: wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(size.x, size.y))
	, m_edit_impl(this, m_sub_mgr)
{
	m_canvas = std::make_unique<Canvas>(this, m_edit_impl);

	Bind(wxEVT_SIZE, &WxMaterialPreview::OnSize, this, GetId());
}

void WxMaterialPreview::OnSize(wxSizeEvent& event)
{
	m_canvas->SetSize(event.GetSize());
}

//////////////////////////////////////////////////////////////////////////
// class WxMaterialPreview::Canvas
//////////////////////////////////////////////////////////////////////////

WxMaterialPreview::Canvas::Canvas(wxWindow* parent, ee0::EditPanelImpl& edit_impl)
	: ee0::WxStageCanvas(parent, edit_impl, std::make_shared<pt3::PerspCam>(sm::vec3(0, 2, -2), sm::vec3(0, 0, 0), sm::vec3(0, 1, 0)), nullptr, nullptr, HAS_3D)
{
}

void WxMaterialPreview::Canvas::OnSize(int w, int h)
{
	auto& wc = pt3::Blackboard::Instance()->GetWindowContext();
	if (wc)
	{
		wc->SetScreen(w, h);

		m_camera->OnSize(static_cast<float>(w), static_cast<float>(h));
		wc->SetProjection(m_camera->GetProjectionMat());
	}
}

void WxMaterialPreview::Canvas::OnDrawSprites() const
{
	auto& ur_rc = const_cast<ee0::RenderContext&>(GetRenderContext()).facade_rc->GetUrRc();
	ur_rc.SetClearFlag(ur::MASKC | ur::MASKD);
	ur_rc.Clear(0x88888888);
	ur_rc.SetDepthTest(ur::DEPTH_LESS_EQUAL);
	ur_rc.EnableDepthMask(true);
	ur_rc.SetFrontFace(true);
	ur_rc.SetCull(ur::CULL_BACK);

	auto& wc = pt3::Blackboard::Instance()->GetWindowContext();
	if (!wc) {
		return;
	}
	wc->SetModelView(m_camera->GetModelViewMat());

	pt3::Material mat;
	mat.ambient = sm::vec3(1, 0, 0);
	mat.shininess = 150;
	pt3::RenderParams params;
	params.mt = sm::mat4::Translated(0, 0, 2);
	pt3::RenderSystem::Instance()->DrawMaterial(mat, params);
}

}