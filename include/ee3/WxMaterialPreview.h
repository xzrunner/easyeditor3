#pragma once

#include <ee0/SubjectMgr.h>
#include <ee0/EditPanelImpl.h>
#include <ee0/WxStageCanvas.h>
#include <ee0/Observer.h>

#include <SM_Vector.h>
#include <painting0/Material.h>
#include <painting3/Viewport.h>

#include <wx/panel.h>

namespace pt0 { class Shader; }
namespace facade { class ImageCube; }

namespace ee3
{

class WxMaterialPreview : public wxPanel
{
public:
	WxMaterialPreview(wxWindow* parent, const sm::ivec2& size, const ee0::SubjectMgrPtr& sub_mgr,
		const ee0::RenderContext* rc, bool user_effect = false);

	pt0::Material& GetMaterial() { return m_material; }

	void RefreshCanvas();

	auto& GetCanvas() const { return m_canvas; }

    void SetShader(const std::shared_ptr<pt0::Shader>& shader) {
        if (m_canvas) {
            m_canvas->SetShader(shader);
        }
    }

    void SetSkybox(const std::shared_ptr<facade::ImageCube>& skybox) {
        m_skybox = skybox;
    }

private:
	void OnSize(wxSizeEvent& event);

private:
	class Canvas : public ee0::WxStageCanvas, public ee0::Observer
	{
	public:
		Canvas(WxMaterialPreview* panel, const ee0::RenderContext* rc, bool user_effect);
		virtual ~Canvas();

		virtual void OnNotify(uint32_t msg, const ee0::VariantSet& variants) override;

		const pt3::Viewport& GetViewport() const { return m_viewport; }

        void SetShader(const std::shared_ptr<pt0::Shader>& shader) {
            m_shader = shader;
        }

	protected:
		virtual void OnSize(int w, int h) override;
		virtual void OnDrawSprites() const override;

    private:
        void DrawSkybox() const;
        void DrawMaterial() const;

	private:
        WxMaterialPreview* m_panel;

		bool m_user_effect;

		pt3::Viewport m_viewport;

        std::shared_ptr<pt0::Shader> m_shader = nullptr;

	}; // Canvas

private:
	ee0::SubjectMgrPtr      m_sub_mgr = nullptr;
	ee0::EditPanelImpl      m_edit_impl;
	std::unique_ptr<Canvas> m_canvas = nullptr;

	pt0::Material m_material;

    std::shared_ptr<facade::ImageCube> m_skybox = nullptr;

}; // WxMaterialPreview

}