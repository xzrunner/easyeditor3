#include "ee3/PolySelectOP.h"
#include "ee3/MeshPointQuery.h"

#include <ee0/WxStagePage.h>
#include <ee0/EditOpState.h>
#include <ee0/SubjectMgr.h>
#include <ee0/color_config.h>
#include <ee0/MsgHelper.h>

#include <node0/SceneNode.h>
#include <node3/CompTransform.h>
#include <unirender/Blackboard.h>
#include <unirender/RenderContext.h>
#include <painting3/PerspCam.h>
#include <painting3/Viewport.h>
#include <painting3/PrimitiveDraw.h>

namespace ee3
{

PolySelectOP::PolySelectOP(ee0::WxStagePage& stage, pt3::PerspCam& cam,
	                       const pt3::Viewport& vp)
	: m_stage(stage)
	, m_cam(cam)
	, m_vp(vp)
	, m_sub_mgr(stage.GetSubjectMgr())
	, m_selection(stage.GetSelection())
{
}

bool PolySelectOP::OnKeyDown(int key_code)
{
	if (ee0::EditOP::OnKeyDown(key_code)) {
		return true;
	}

	if (key_code == WXK_SHIFT && m_selected.poly) {
		m_move_select = true;
	}

	return false;
}

bool PolySelectOP::OnKeyUp(int key_code)
{
	if (ee0::EditOP::OnKeyUp(key_code)) {
		return true;
	}

	if (key_code == WXK_SHIFT)
	{
		m_move_select = false;
		m_selected_face.clear();

		m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
	}

	return false;
}

bool PolySelectOP::OnMouseLeftDown(int x, int y)
{
	MeshPointQuery::Selected selected;

	ee0::GameObj selected_obj = GAME_OBJ_NULL;
	sm::vec3 ray_dir = m_vp.TransPos3ScreenToDir(
		sm::vec2(static_cast<float>(x), static_cast<float>(y)), m_cam);
	sm::Ray ray(m_cam.GetPos(), ray_dir);
	m_stage.Traverse([&](const ee0::GameObj& obj)->bool
	{
		bool find = MeshPointQuery::Query(obj, ray, m_cam.GetPos(), selected);
		if (find) {
			selected_obj = obj;
		}
		return !find;
	});

	if (GAME_OBJ_VALID(selected_obj))
	{
		m_selected = selected;

		// insert to selection set
		std::vector<n0::NodeWithPos> nwps;
		nwps.push_back(n0::NodeWithPos(selected_obj, selected_obj, 0));
		ee0::MsgHelper::InsertSelection(*m_sub_mgr, nwps);

		UpdatePolyBorderPos();
	}
	else
	{
		if (m_select_null) {
			m_selected.Reset();
			m_selected_poly.clear();
			m_selected_face.clear();
		}
	}

	if (ee0::EditOP::OnMouseLeftDown(x, y))
	{
		return true;
	}

	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

	return false;
}

bool PolySelectOP::OnMouseMove(int x, int y)
{
	if (ee0::EditOP::OnMouseMove(x, y)) {
		return true;
	}

	if (!m_move_select || !m_selected.poly) {
		return false;
	}

	assert(m_selected.node);
	auto& ctrans = m_selected.node->GetUniqueComp<n3::CompTransform>();

	sm::vec3 ray_dir = m_vp.TransPos3ScreenToDir(
		sm::vec2(static_cast<float>(x), static_cast<float>(y)), m_cam);
	sm::Ray ray(m_cam.GetPos(), ray_dir);

	m_selected.min_dist = std::numeric_limits<float>::max();
	MeshPointQuery::Query(m_selected.poly, ctrans, ray, m_cam.GetPos(), m_selected);

	if (m_selected.face) {
		m_selected_face.clear();
		m_selected.face->GetBorder(m_selected_face);
	}

	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

	return false;
}

bool PolySelectOP::OnDraw() const
{
	if (ee0::EditOP::OnDraw()) {
		return true;
	}

	if (!m_selected.poly || !m_selected.face) {
		return false;
	}

	auto& ur_rc = ur::Blackboard::Instance()->GetRenderContext();
	ur_rc.SetDepthTest(ur::DEPTH_DISABLE);
	ur_rc.EnableDepthMask(false);

	// poly
	pt3::PrimitiveDraw::SetColor(ee0::LIGHT_RED.ToABGR());
	for (auto& face : m_selected_poly) {
		pt3::PrimitiveDraw::Polyline(face, true);
	}

	// face
	pt3::PrimitiveDraw::SetColor(ee0::LIGHT_RED.ToABGR());
	if (m_move_select && m_selected.face && !m_selected_face.empty()) {
		pt3::PrimitiveDraw::Polygon(m_selected_face);
	}

	ur_rc.SetDepthTest(ur::DEPTH_LESS_EQUAL);
	ur_rc.EnableDepthMask(true);

	return false;
}

void PolySelectOP::UpdatePolyBorderPos()
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