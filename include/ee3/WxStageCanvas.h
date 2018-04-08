#pragma once

#include <ee0/WxStageCanvas.h>
#include <ee0/Observer.h>
#include <ee0/GameObj.h>

#include <painting3/Camera.h>
#include <painting3/Viewport.h>

namespace ee0 { class WxLibraryPanel; class RenderContext; class WxStagePage; }

namespace ee3
{

class WxStageCanvas : public ee0::WxStageCanvas, public ee0::Observer
{
public:
	WxStageCanvas(ee0::WxStagePage* stage, const ee0::RenderContext* rc = nullptr, 
		const ee0::WindowContext* wc = nullptr, bool has2d = false);
	virtual ~WxStageCanvas();

	virtual void OnNotify(uint32_t msg, const ee0::VariantSet& variants) override;

	pt3::Camera& GetCamera() { return m_camera; }
	const pt3::Camera& GetCamera() const { return m_camera; }

	const pt3::Viewport& GetViewport() const { return m_viewport; }

	//sm::vec2 TransPos3ProjectToScreen(const sm::vec3& proj) const;
	//sm::vec3 TransPos3ScreenToDir(const sm::vec2& screen) const;

protected:
	virtual void OnSize(int w, int h) override;
	virtual void OnDrawSprites() const override;

	virtual void DrawBackground() const;

private:
	void DrawNodes() const;

private:
	ee0::WxStagePage* m_stage;

	bool m_has2d;

	pt3::Camera   m_camera;
	pt3::Viewport m_viewport;

	sm::mat4 m_mat_projection;

}; // WxStageCanvas

}