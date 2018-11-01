#include "ee3/WxStageCanvas.h"

#include <ee0/color_config.h>
#include <ee0/EditOP.h>
#include <ee0/WxStagePage.h>
#include <ee0/SubjectMgr.h>
#include <ee0/RenderContext.h>

#include <tessellation/Painter.h>
#include <painting2/Blackboard.h>
#include <painting2/WindowContext.h>
#include <painting2/RenderSystem.h>
#include <painting3/Blackboard.h>
#include <painting3/WindowContext.h>
#include <painting3/PerspCam.h>
#ifndef GAME_OBJ_ECS
#include <node3/RenderSystem.h>
#endif // GAME_OBJ_ECS

namespace
{

const uint32_t MESSAGES[] =
{
	ee0::MSG_SET_CANVAS_DIRTY,
};

}

namespace ee3
{

WxStageCanvas::WxStageCanvas(ee0::WxStagePage* stage, const ee0::RenderContext* rc,
	                         const ee0::WindowContext* wc, bool has2d)
	: ee0::WxStageCanvas(stage, stage->GetImpl(), std::make_shared<pt3::PerspCam>(sm::vec3(0, 2, -2), sm::vec3(0, 0, 0), sm::vec3(0, 1, 0)), rc, wc, HAS_2D * has2d | HAS_3D)
	, m_stage(stage)
	, m_has2d(has2d)
{
	for (auto& msg : MESSAGES) {
		stage->GetSubjectMgr()->RegisterObserver(msg, this);
	}
}

WxStageCanvas::~WxStageCanvas()
{
	for (auto& msg : MESSAGES) {
		m_stage->GetSubjectMgr()->UnregisterObserver(msg, this);
	}
}

void WxStageCanvas::OnNotify(uint32_t msg, const ee0::VariantSet& variants)
{
	switch (msg)
	{
	case ee0::MSG_SET_CANVAS_DIRTY:
		SetDirty();
		break;
	}
}

//sm::vec2 WxStageCanvas::TransPos3ProjectToScreen(const sm::vec3& proj) const
//{
//	return m_viewport.TransPos3ProjectToScreen(m_mat_projection * GetCamera().GetViewMat() * proj, m_camera);
//}
//
//sm::vec3 WxStageCanvas::TransPos3ScreenToDir(const sm::vec2& screen) const
//{
//	return m_viewport.TransPos3ScreenToDir(screen, m_camera);
//}

void WxStageCanvas::OnSize(int w, int h)
{
	auto& wc = pt3::Blackboard::Instance()->GetWindowContext();
	if (wc)
	{
		wc->SetScreen(w, h);
		m_viewport.SetSize(w, h);

		m_camera->OnSize(static_cast<float>(w), static_cast<float>(h));
		wc->SetProjection(m_camera->GetProjectionMat());
	}

	if (m_has2d)
	{
		auto& wc = pt2::Blackboard::Instance()->GetWindowContext();
		if (wc)
		{
			wc->SetScreen(w, h);
			wc->SetProjection(w, h);
		}
	}
}

void WxStageCanvas::OnDrawSprites() const
{
	ee0::RenderContext::Reset3D(true);

	auto& wc = pt3::Blackboard::Instance()->GetWindowContext();
	if (!wc) {
		return;
	}
	wc->SetView(m_camera->GetViewMat());

	if (m_has2d)
	{
		auto& wc = pt2::Blackboard::Instance()->GetWindowContext();
		if (wc) {
			wc->SetView(sm::vec2(0, 0), 1);
		}
	}

	DrawBackground();
	DrawForeground();

	auto edit_op = m_stage->GetImpl().GetEditOP();
	if (edit_op) {
		ee0::RenderContext::Reset2D();
		edit_op->OnDraw();
	}
}

void WxStageCanvas::DrawBackground() const
{
	tess::Painter pt;

	auto cam_mat = m_camera->GetViewMat() * m_camera->GetProjectionMat();
	auto trans3d = [&](const sm::vec3& pos3)->sm::vec2 {
		return m_viewport.TransPosProj3ToProj2(pos3, cam_mat);
	};

	// draw cross
	uint32_t col = 0xff000088;
	const float len = 50.0f;
	pt.AddLine3D({ -len, 0, 0 }, { len, 0, 0 }, trans3d, col);
	pt.AddLine3D({ 0, -len, 0 }, { 0, len, 0 }, trans3d, col);
	pt.AddLine3D({ 0, 0, -len }, { 0, 0, len }, trans3d, col);

	static const int TOT_LEN = 100;
	static const int GRID_EDGE = 5;
	for (int z = -TOT_LEN; z < TOT_LEN; z += GRID_EDGE) {
		for (int x = -TOT_LEN; x < TOT_LEN; x += GRID_EDGE) {
			if ((x + z) % (GRID_EDGE * 2) == 0) {
				col = 0xff444444;
			} else {
				col = 0xff888888;
			}
			sm::vec3 polygon[4] = {
				sm::vec3(x, 0, z),
				sm::vec3(x + GRID_EDGE, 0, z),
				sm::vec3(x + GRID_EDGE, 0, z + GRID_EDGE),
				sm::vec3(x, 0, z + GRID_EDGE)
			};
			pt.AddPolygonFilled3D(polygon, 4, trans3d, col);
		}
	}

	pt2::RenderSystem::DrawPainter(pt);
}

void WxStageCanvas::DrawForeground() const
{
	DrawNodes();
}

void WxStageCanvas::DrawNodes(pt3::RenderParams::DrawType type) const
{
	ee0::VariantSet vars;
	ee0::Variant var;
	var.m_type = ee0::VT_LONG;
	var.m_val.l = ee0::WxStagePage::TRAV_DRAW;
	vars.SetVariant("type", var);

	pt3::RenderParams params;
	params.mt = m_camera->GetViewMat();
	params.type = type;

	m_stage->Traverse([&](const ee0::GameObj& obj)->bool {
#ifndef GAME_OBJ_ECS
		n3::RenderSystem::Draw(obj, params);
#endif // GAME_OBJ_ECS
		return true;
	}, vars);
}

}