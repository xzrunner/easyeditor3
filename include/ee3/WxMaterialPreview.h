#pragma once

#include <ee0/SubjectMgr.h>
#include <ee0/EditPanelImpl.h>
#include <ee0/WxStageCanvas.h>
#include <ee0/Observer.h>

#include <SM_Vector.h>
#include <painting3/Material.h>
#include <painting3/Viewport.h>

#include <wx/panel.h>

namespace ee3
{

class WxMaterialPreview : public wxPanel
{
public:
	WxMaterialPreview(wxWindow* parent, const sm::ivec2& size,
		const ee0::RenderContext* rc);

	pt3::Material& GetMaterial() { return m_material; }

	void RefreshCanvas();

private:
	void OnSize(wxSizeEvent& event);

private:
	class Canvas : public ee0::WxStageCanvas, public ee0::Observer
	{
	public:
		Canvas(wxWindow* parent, ee0::EditPanelImpl& edit_impl,
			const ee0::RenderContext* rc, const ee0::SubjectMgrPtr& sub_mgr,
			const pt3::Material& material);
		virtual ~Canvas();

		virtual void OnNotify(uint32_t msg, const ee0::VariantSet& variants) override;

		const pt3::Viewport& GetViewport() const { return m_viewport; }

	protected:
		virtual void OnSize(int w, int h) override;
		virtual void OnDrawSprites() const override;

	private:
		ee0::SubjectMgrPtr   m_sub_mgr;
		const pt3::Material& m_material;

		pt3::Viewport m_viewport;

	}; // Canvas

private:
	ee0::SubjectMgrPtr      m_sub_mgr = std::make_shared<ee0::SubjectMgr>();
	ee0::EditPanelImpl      m_edit_impl;
	std::unique_ptr<Canvas> m_canvas = nullptr;

	pt3::Material m_material;

}; // WxMaterialPreview

}