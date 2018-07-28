#include "ee3/PolySelectOP.h"
#include "ee3/MeshPointQuery.h"

#include <ee0/WxStagePage.h>
#include <ee0/EditOpState.h>
#include <ee0/SubjectMgr.h>
#include <ee0/color_config.h>

#include <unirender/Blackboard.h>
#include <unirender/RenderContext.h>
#include <painting3/Camera.h>
#include <painting3/Viewport.h>
#include <painting3/PrimitiveDraw.h>

namespace ee3
{

PolySelectOP::PolySelectOP(ee0::WxStagePage& stage, pt3::Camera& cam,
	                       const pt3::Viewport& vp)
	: ee3::WorldTravelOP(cam, vp, stage.GetSubjectMgr())
	, m_stage(stage)
	, m_vp(vp)
	, m_selection(stage.GetSelection())
{
}

bool PolySelectOP::OnMouseLeftDown(int x, int y)
{
	bool intersect = false;
	sm::vec3 ray_dir = m_vp.TransPos3ScreenToDir(
		sm::vec2(static_cast<float>(x), static_cast<float>(y)), m_cam);
	sm::Ray ray(m_cam.GetPos(), ray_dir);
	m_stage.Traverse([&](const ee0::GameObj& obj)->bool
	{
		bool find = MeshPointQuery::Query(obj, ray, m_cam.GetPos(), m_selected);
		if (find) {
			intersect = true;
		}
		return !find;
	});

	if (intersect)
	{
		CachePolyBorderPos();
	}
	else
	{
		m_selected.Reset();
		m_selected_poly.clear();
		if (WorldTravelOP::OnMouseLeftDown(x, y)) {
			return true;
		}
	}

	if (m_op_state->OnMousePress(x, y)) {
		return true;
	}

	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

	return false;
}

bool PolySelectOP::OnDraw() const
{
	if (WorldTravelOP::OnDraw()) {
		return true;
	}

	if (!m_selected.poly || !m_selected.face) {
		return false;
	}

	auto& ur_rc = ur::Blackboard::Instance()->GetRenderContext();
	ur_rc.SetDepthTest(ur::DEPTH_DISABLE);
	ur_rc.EnableDepthMask(false);
	pt3::PrimitiveDraw::SetColor(ee0::LIGHT_RED.ToABGR());
	for (auto& face : m_selected_poly) {
		pt3::PrimitiveDraw::Polyline(face, true);
	}
	ur_rc.SetDepthTest(ur::DEPTH_LESS_EQUAL);
	ur_rc.EnableDepthMask(true);

	return false;
}

void PolySelectOP::CachePolyBorderPos()
{
	if (!m_selected.poly) {
		return;
	}

	m_selected_poly.clear();
	auto& faces = m_selected.poly->GetFaces();
	m_selected_poly.reserve(faces.size());
	for (auto& face : faces)
	{
		std::vector<sm::vec3> border;
		face->GetBorder(border);
		m_selected_poly.push_back(border);
	}
}

}