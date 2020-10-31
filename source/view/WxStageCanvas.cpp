#include "ee3/WxStageCanvas.h"

#include <ee0/EditOP.h>
#include <ee0/WxStagePage.h>
#include <ee0/SubjectMgr.h>

#undef DrawState

#include <tessellation/Painter.h>
#include <unirender/Factory.h>
#include <unirender/DrawState.h>
#include <unirender/ClearState.h>
#include <unirender/Context.h>
#include <painting2/RenderSystem.h>
#include <painting3/WindowContext.h>
#include <painting3/PerspCam.h>
#include <painting3/MaterialMgr.h>
#include <painting3/PointLight.h>
#include <renderpipeline/Utility.h>
//#ifndef GAME_OBJ_ECS
#include <node0/SceneNode.h>
#include <node3/RenderSystem.h>
#include <node3/CompLight.h>
#include <node3/CompTransform.h>
//#endif // GAME_OBJ_ECS
#include <facade/ImageCube.h>

namespace
{

const uint32_t MESSAGES[] =
{
	ee0::MSG_SET_CANVAS_DIRTY,
};

sm::vec3 DEFAULT_CAM_POS(0, 2, -2);
sm::vec3 DEFAULT_CAM_TARGET(0, 0, 0);
sm::vec3 DEFAULT_CAM_UP(0, 1, 0);

}

namespace ee3
{

WxStageCanvas::WxStageCanvas(const ur::Device& dev, ee0::WxStagePage* stage, const ee0::RenderContext* rc,
	                         const ee0::WindowContext* wc, bool has2d)
	: ee0::WxStageCanvas(dev, stage, stage->GetImpl(), std::make_shared<pt3::PerspCam>(DEFAULT_CAM_POS, DEFAULT_CAM_TARGET, DEFAULT_CAM_UP), rc, wc, HAS_2D * has2d | HAS_3D)
	, m_stage(stage)
	, m_has2d(has2d)
{
	for (auto& msg : MESSAGES) {
		stage->GetSubjectMgr()->RegisterObserver(msg, this);
	}

    std::static_pointer_cast<pt3::PerspCam>(m_camera)->SetWndCtx(GetWidnowContext().wc3);
}

WxStageCanvas::~WxStageCanvas()
{
	for (auto& msg : MESSAGES) {
		m_stage->GetSubjectMgr()->UnregisterObserver(msg, this);
	}
}

void WxStageCanvas::ResetCamera()
{
    auto cam = GetCamera();
    assert(cam->TypeID() == pt0::GetCamTypeID<pt3::PerspCam>());
    auto p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);
    p_cam->SetPosAndAngle(DEFAULT_CAM_POS, DEFAULT_CAM_TARGET, DEFAULT_CAM_UP);
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

void WxStageCanvas::SetSkybox(const std::shared_ptr<facade::ImageCube>& skybox)
{
    m_skybox = skybox;

    if (m_skybox && m_skybox->GetTexture()) {
        rp::InitGIWithSkybox(m_dev, *GetRenderContext().ur_ctx, m_skybox->GetTexture(), m_gi);
    }
}

void WxStageCanvas::OnSize(int w, int h)
{
    m_viewport.SetSize(w, h);
}

void WxStageCanvas::OnDrawSprites() const
{
    ur::ClearState clear;
    clear.buffers = ur::ClearBuffers::ColorAndDepthBuffer;
    clear.color.FromRGBA(m_bg_color);
    GetRenderContext().ur_ctx->Clear(clear);

	ee0::RenderContext::Reset3D(true);

	//auto& wc = pt3::Blackboard::Instance()->GetWindowContext();
	//if (!wc) {
	//	return;
	//}
	//wc->SetView(m_camera->GetViewMat());

	DrawBackground3D();
	DrawForeground3D();

    ee0::RenderContext::Reset2D();

    DrawBackground2D();
    DrawForeground2D();

	auto edit_op = m_stage->GetImpl().GetEditOP();
	if (edit_op) {
		edit_op->OnDraw(m_dev, *GetRenderContext().ur_ctx);
	}
}

//// use pt2
//void WxStageCanvas::DrawBackground() const
//{
//	tess::Painter pt;
//
//	auto cam_mat = m_camera->GetProjectionMat() * m_camera->GetViewMat();
//	auto trans3d = [&](const sm::vec3& pos3)->sm::vec2 {
//		return m_viewport.TransPosProj3ToProj2(pos3, cam_mat);
//	};
//
//	//// draw cross
//	//uint32_t col = 0xff000088;
//	//const float len = 50.0f;
//	//pt.AddLine3D({ -len, 0, 0 }, { len, 0, 0 }, trans3d, col);
//	//pt.AddLine3D({ 0, -len, 0 }, { 0, len, 0 }, trans3d, col);
//	//pt.AddLine3D({ 0, 0, -len }, { 0, 0, len }, trans3d, col);
//
// //   // draw grids
//	//static const int TOT_LEN = 100;
//	//static const int GRID_EDGE = 5;
//	//for (int z = -TOT_LEN; z < TOT_LEN; z += GRID_EDGE) {
//	//	for (int x = -TOT_LEN; x < TOT_LEN; x += GRID_EDGE) {
//	//		if ((x + z) % (GRID_EDGE * 2) == 0) {
//	//			col = 0xff444444;
//	//		} else {
//	//			col = 0xff888888;
//	//		}
//	//		sm::vec3 polygon[4] = {
//	//			sm::vec3(x, 0, z),
//	//			sm::vec3(x + GRID_EDGE, 0, z),
//	//			sm::vec3(x + GRID_EDGE, 0, z + GRID_EDGE),
//	//			sm::vec3(x, 0, z + GRID_EDGE)
//	//		};
//	//		pt.AddPolygonFilled3D(polygon, 4, trans3d, col);
//	//	}
//	//}
//
//    // draw cross
//    uint32_t col = 0xff000000;
//    const float len = 50.0f;
//    pt.AddLine3D({ -len, 0, 0 }, { len, 0, 0 }, trans3d, col);
//    pt.AddLine3D({ 0, 0, -len }, { 0, 0, len }, trans3d, col);
//
//    // draw grids
//    col = 0xffaaaaaa;
//    static const float TOT_LEN = 100;
//    static const float GRID_EDGE = 5;
//    for (float z = -TOT_LEN; z <= TOT_LEN; z += GRID_EDGE) {
//        pt.AddLine3D({ -TOT_LEN, 0, z }, { TOT_LEN, 0, z }, trans3d, col);
//    }
//    for (float x = -TOT_LEN; x <= TOT_LEN; x += GRID_EDGE) {
//        pt.AddLine3D({ x, 0, -TOT_LEN }, { x, 0, TOT_LEN }, trans3d, col);
//    }
//
//	pt2::RenderSystem::DrawPainter(pt);
//}

void WxStageCanvas::DrawBackgroundGrids(float tot_len, float grid_edge) const
{
    std::vector<sm::vec3> buf;
    uint32_t col = 0xff000000;
    buf.reserve(tot_len * 2 / grid_edge * 2);
    for (float z = -tot_len; z <= tot_len; z += grid_edge) {
        buf.push_back({ -tot_len, 0, z });
        buf.push_back({  tot_len, 0, z });
    }
    for (float x = -tot_len; x <= tot_len; x += grid_edge) {
        buf.push_back({ x, 0, -tot_len });
        buf.push_back({ x, 0, tot_len });
    }
    pt3::RenderSystem::DrawLines3D(m_dev, *GetRenderContext().ur_ctx, buf.size(), buf[0].xyz, col);
}

void WxStageCanvas::DrawBackgroundCross() const
{
	tess::Painter pt;

	auto cam_mat = m_camera->GetProjectionMat() * m_camera->GetViewMat();
	auto trans3d = [&](const sm::vec3& pos3)->sm::vec2 {
		return GetViewport().TransPosProj3ToProj2(pos3, cam_mat);
	};

	const float len = 1;
	pt.AddLine3D({ -len, 0, 0 }, { len, 0, 0 }, trans3d, 0xff0000ff, 2);
	pt.AddLine3D({ 0, -len, 0 }, { 0, len, 0 }, trans3d, 0xff00ff00, 2);
	pt.AddLine3D({ 0, 0, -len }, { 0, 0, len }, trans3d, 0xffff0000, 2);

	const float radius = 5.0f;
	pt.AddCircleFilled(trans3d(sm::vec3(len, 0, 0)), radius, 0xff0000ff);
	pt.AddCircleFilled(trans3d(sm::vec3(0, len, 0)), radius, 0xff00ff00);
	pt.AddCircleFilled(trans3d(sm::vec3(0, 0, len)), radius, 0xffff0000);

    auto rs = ur::DefaultRenderState2D();
	pt2::RenderSystem::DrawPainter(m_dev, *GetRenderContext().ur_ctx, rs, pt);
}

void WxStageCanvas::DrawNodes(bool draw_mesh_border) const
{
	ee0::VariantSet vars;
	ee0::Variant var;
	var.m_type = ee0::VT_LONG;
	var.m_val.l = ee0::WxStagePage::TRAV_DRAW;
	vars.SetVariant("type", var);

	pt3::RenderParams params;
    if (draw_mesh_border) {
        params.mask.set(pt3::RenderParams::DrawMeshBorder);
    }

    pt0::RenderContext ctx;
    sm::vec3 light_pos(0, 2, -4);
    // calc PointLight's pos
    m_stage->Traverse([&](const ee0::GameObj& obj)->bool
    {
        if (!obj->HasUniqueComp<n3::CompLight>()) {
            return true;
        }

        auto& light = obj->GetUniqueComp<n3::CompLight>().GetLight();
        assert(light->get_type() == rttr::type::get<pt3::PointLight>());
        light_pos = static_cast<const pt3::PointLight*>(light.get())->GetPosition();
        if (obj->HasUniqueComp<n3::CompTransform>()) {
            auto& ctrans = obj->GetUniqueComp<n3::CompTransform>();
            light_pos = ctrans.GetTransformMat() * light_pos;
        }

        return false;
    });
    ctx.AddVar(pt3::MaterialMgr::PositionUniforms::light_pos.name, pt0::RenderVariant(light_pos));

    //auto& wc = pt3::Blackboard::Instance()->GetWindowContext();
    //assert(wc);
    //ctx.AddVar(
    //    pt3::MaterialMgr::PosTransUniforms::view.name,
    //    pt0::RenderVariant(wc->GetViewMat())
    //);
    //ctx.AddVar(
    //    pt3::MaterialMgr::PosTransUniforms::projection.name,
    //    pt0::RenderVariant(wc->GetProjMat())
    //);

    ur::DrawState ds;

	m_stage->Traverse([&](const ee0::GameObj& obj)->bool {
#ifndef GAME_OBJ_ECS
		n3::RenderSystem::Draw(m_dev, *GetRenderContext().ur_ctx, ds, *obj, params, ctx);
#endif // GAME_OBJ_ECS
		return true;
	}, vars);
}

void WxStageCanvas::DrawSkybox() const
{
    if (m_skybox) {
        auto tex = m_skybox->GetTexture();
        if (tex) {
            pt3::RenderSystem::DrawSkybox(m_dev, *GetRenderContext().ur_ctx, *tex);
        }
    }
}

}