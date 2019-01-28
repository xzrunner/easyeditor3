#include "ee3/MeshIKOP.h"

#include <ee0/WxStageCanvas.h>

#include <SM_Ray.h>
#include <SM_RayIntersect.h>
#include <model/MeshIK.h>
#include <model/Model.h>
#include <painting2/RenderSystem.h>
#include <painting3/Blackboard.h>
#include <painting3/WindowContext.h>
#include <painting3/PerspCam.h>
#include <painting3/Viewport.h>
#include <facade/Facade.h>
#include <easygui/Context.h>
#include <easygui/ImGui.h>
#include <tessellation/Painter.h>

namespace
{

sm::vec3 debug_draw_tri[3];

}

namespace ee3
{

MeshIKOP::MeshIKOP(const std::shared_ptr<pt0::Camera>& camera,
	               const std::shared_ptr<ee0::WxStageCanvas>& canvas,
	               const pt3::Viewport& vp)
	: ee0::EditOP(camera)
	, m_canvas(canvas)
	, m_vp(vp)
{
}

bool MeshIKOP::OnMouseLeftDown(int x, int y)
{
	if (ee0::EditOP::OnMouseLeftDown(x, y)) {
		return true;
	}

	QueryByPos(x, y);

	return false;
}

bool MeshIKOP::OnMouseRightDown(int x, int y)
{
	if (ee0::EditOP::OnMouseLeftUp(x, y)) {
		return true;
	}

	if (m_ik) {
		m_ik->Deform(sm::vec3(0, 20, 0));
	}

	return false;
}

bool MeshIKOP::OnMouseRightUp(int x, int y)
{
	if (ee0::EditOP::OnMouseLeftUp(x, y)) {
		return true;
	}

	if (m_ik) {
		m_ik->Deform(sm::vec3(0, -20, 0));
	}

	return false;
}

bool MeshIKOP::OnMouseDrag(int x, int y)
{
	if (ee0::EditOP::OnMouseDrag(x, y)) {
		return true;
	}

	return false;
}

//bool MeshIKOP::OnMouseRightDown(int x, int y)
//{
//	if (ee0::EditOP::OnMouseRightDown(x, y)) {
//		return true;
//	}
//
//	if (m_ik) {
//		m_ik->Deform(sm::vec3(0, -20, 0));
//	}
//
//	return false;
//}
//
//bool MeshIKOP::OnMouseLeftDClick(int x, int y)
//{
//	if (ee0::EditOP::OnMouseLeftDClick(x, y)) {
//		return true;
//	}
//
//	if (m_ik) {
//		m_ik->Deform(sm::vec3(0, 20, 0));
//	}
//
//	return false;
//}

bool MeshIKOP::OnDraw() const
{
	if (ee0::EditOP::OnDraw()) {
		return true;
	}

	DrawGUI();
	DebugDraw();

	return false;
}

bool MeshIKOP::Update(float dt)
{
	if (ee0::EditOP::Update(dt)) {
		return true;
	}

	return false;
}

void MeshIKOP::SetModel(const std::shared_ptr<model::Model>& model)
{
	m_model = model;
}

void MeshIKOP::PrepareDeform() const
{
	if (!m_model->meshes.empty()) {
		m_ik = std::make_unique<model::MeshIK>(m_model->meshes[0]->geometry);
		m_ik->PrepareDeform(m_main_handle, m_handle_region_size, m_unconstrained_region_size);
	}
}

void MeshIKOP::QueryByPos(int x, int y)
{
	auto cam_type = m_camera->TypeID();
	if (cam_type != pt0::GetCamTypeID<pt3::PerspCam>()) {
		return;
	}

	sm::Ray ray;

	auto p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);
	ray.dir = m_vp.TransPos3ScreenToDir(
		sm::vec2(static_cast<float>(x), static_cast<float>(y)), *p_cam);
	ray.origin = p_cam->GetPos();
	auto& cam_pos = p_cam->GetPos();

	float min_dis = std::numeric_limits<float>::max();
	for (auto& mesh : m_model->meshes)
	{
		auto& data = mesh->geometry.raw_data;
		for (auto& face : data->faces)
		{
			sm::vec3 intersect;

			const float scale = 0.01f;
			auto v0 = data->vertices[face[0]] * scale;
			auto v1 = data->vertices[face[1]] * scale;
			auto v2 = data->vertices[face[2]] * scale;
			if (sm::ray_triangle_intersect(sm::mat4(), v0, v1, v2, ray, &intersect))
			{
				m_main_handle = face[0];
				PrepareDeform();

				debug_draw_tri[0] = v0;
				debug_draw_tri[1] = v1;
				debug_draw_tri[2] = v2;
				return;
			}
		}
	}
}

void MeshIKOP::DrawGUI() const
{
    auto& ctx = m_canvas->GetWidnowContext().egui;
    if (!ctx) {
        return;
    }

	auto& wc = pt3::Blackboard::Instance()->GetWindowContext();
	auto sz = wc->GetScreenSize();
	m_canvas->PrepareDrawGui(sz.x, sz.y);

	int uid = 1;

	ctx->BeginDraw();

	bool dirty = facade::Facade::Instance()->IsLastFrameDirty();
	//if (m_last_screen_sz != sz) {
	//	dirty = true;
	//	m_last_screen_sz = sz;
	//}
	if (egui::slider(uid++, "handle_region_size", &m_handle_region_size, sz.x * 0.5f - 60, sz.y * 0.5f - 140, 128, 20, true, *ctx, dirty)) {
		PrepareDeform();
	}
	if (egui::slider(uid++, "unconstrained_region_size", &m_unconstrained_region_size, sz.x * 0.5f - 30, sz.y * 0.5f - 140, 128, 40, true, *ctx, dirty)) {
		PrepareDeform();
	}

	ctx->EndDraw();

	// todo
	const float dt = 0.033f;
	ctx->Update(dt);
}

void MeshIKOP::DebugDraw() const
{
	auto cam_mat = m_camera->GetViewMat() * m_camera->GetProjectionMat();

	sm::vec2 tri[3];
	for (size_t i = 0; i < 3; ++i) {
		tri[i] = m_vp.TransPosProj3ToProj2(debug_draw_tri[i], cam_mat);
	}

	tess::Painter pt;
	pt.AddTriangleFilled(tri[0], tri[1], tri[2], 0xff0000ff);
	pt2::RenderSystem::DrawPainter(pt);
}

}