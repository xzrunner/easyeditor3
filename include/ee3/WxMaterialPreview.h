#pragma once

#include <ee0/SubjectMgr.h>
#include <ee0/EditPanelImpl.h>
#include <ee0/WxStageCanvas.h>
#include <ee0/Observer.h>

#include <SM_Vector.h>
#include <painting0/Material.h>
#include <painting3/Viewport.h>
#include <painting3/GlobalIllumination.h>

#include <wx/panel.h>

namespace facade { class ImageCube; }

namespace ee3
{

class WxMaterialPreview : public wxPanel
{
public:
	WxMaterialPreview(const ur::Device& dev, ur::Context& ctx, wxWindow* parent,
        const sm::ivec2& size, const ee0::SubjectMgrPtr& sub_mgr,
		const ee0::RenderContext* rc, bool user_effect = false);

	pt0::Material& GetMaterial() { return m_material; }

	void RefreshCanvas();

	auto& GetCanvas() const { return m_canvas; }

    void SetShader(const std::shared_ptr<ur::ShaderProgram>& shader) {
        if (m_canvas) {
            m_canvas->SetShader(shader);
        }
    }

    void SetSkybox(const std::shared_ptr<facade::ImageCube>& skybox);
    auto& GetGlobalIllumination() const { return m_gi; }

private:
	void OnSize(wxSizeEvent& event);

private:
	class Canvas : public ee0::WxStageCanvas, public ee0::Observer
	{
	public:
		Canvas(const ur::Device& dev, ur::Context& ctx, WxMaterialPreview* panel,
            const ee0::RenderContext* rc, bool user_effect);
		virtual ~Canvas();

		virtual void OnNotify(uint32_t msg, const ee0::VariantSet& variants) override;

		const pt3::Viewport& GetViewport() const { return m_viewport; }

        void SetShader(const std::shared_ptr<ur::ShaderProgram>& shader) {
            m_shader = shader;
        }

	protected:
		virtual void OnDrawSprites() const override;

    private:
        void DrawSkybox() const;
        void DrawMaterial() const;

	private:
        const ur::Device& m_dev;
        ur::Context& m_ctx;

        WxMaterialPreview* m_panel;

		bool m_user_effect;

		pt3::Viewport m_viewport;

        std::shared_ptr<ur::ShaderProgram> m_shader = nullptr;

	}; // Canvas

private:
    const ur::Device& m_dev;
    ur::Context& m_ctx;

	ee0::SubjectMgrPtr      m_sub_mgr = nullptr;
	ee0::EditPanelImpl      m_edit_impl;
	std::unique_ptr<Canvas> m_canvas = nullptr;

	pt0::Material m_material;

    std::shared_ptr<facade::ImageCube> m_skybox = nullptr;
    pt3::GlobalIllumination m_gi;

}; // WxMaterialPreview

}