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
#include <painting3/OrthoCam.h>
#include <painting3/Viewport.h>
#include <painting3/PrimitiveDraw.h>

namespace
{

const bool PRESS_SELECT = false;

}

namespace ee3
{
namespace mesh
{

PolySelectOP::PolySelectOP(const std::shared_ptr<pt0::Camera>& camera,
                           ee0::WxStagePage& stage,
	                       const pt3::Viewport& vp)
	: ee0::EditOP(camera)
	, m_stage(stage)
	, m_vp(vp)
	, m_sub_mgr(stage.GetSubjectMgr())
	, m_selection(stage.GetSelection())
{
	m_first_pos.MakeInvalid();
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
	if (ee0::EditOP::OnMouseLeftDown(x, y))
	{
		return true;
	}

	// select
	if (PRESS_SELECT)
	{
		SelectByPos(sm::ivec2(x, y), m_selected);
		m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
	}
	else
	{
		// test if cancel selected
		if (m_selected.poly)
		{
			MeshPointQuery::Selected selected;
			SelectByPos(sm::ivec2(x, y), selected);
			if (selected.poly != m_selected.poly) {
				ClearSelected();
			} else {
				m_selected = selected;
			}
		}
	}

	m_first_pos.Set(x, y);

	return false;
}

bool PolySelectOP::OnMouseLeftUp(int x, int y)
{
	if (ee0::EditOP::OnMouseLeftUp(x, y)) {
		return true;
	}

	// select
	if (!PRESS_SELECT)
	{
		if (m_first_pos == sm::ivec2(x, y))
		{
			SelectByPos(m_first_pos, m_selected);
			m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
		}
	}

	m_first_pos.MakeInvalid();

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

	if (m_camera->TypeID() == pt0::GetCamTypeID<pt3::PerspCam>())
	{
		auto p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);

		assert(m_selected.node);
		auto& ctrans = m_selected.node->GetUniqueComp<n3::CompTransform>();

		sm::vec3 ray_dir = m_vp.TransPos3ScreenToDir(
			sm::vec2(static_cast<float>(x), static_cast<float>(y)), *p_cam);
		sm::Ray ray(p_cam->GetPos(), ray_dir);

		m_selected.min_dist = std::numeric_limits<float>::max();
		MeshPointQuery::Query(m_selected.poly, ctrans, ray, p_cam->GetPos(), m_selected);

		if (m_selected.face) {
			m_selected_face.clear();
			m_selected.face->GetBorder(m_selected_face);
		}

		m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
	}

	return false;
}

bool PolySelectOP::OnMouseDrag(int x, int y)
{
	if (ee0::EditOP::OnMouseDrag(x, y)) {
		return true;
	}

	//if (sm::ivec2(x, y) != m_first_pos) {
	//	ClearSelected();
	//}

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

void PolySelectOP::UpdateCachedPolyBorder()
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

void PolySelectOP::SelectByPos(const sm::ivec2& pos, MeshPointQuery::Selected& selected)
{
	selected.Reset();

	ee0::GameObj selected_obj = GAME_OBJ_NULL;

	sm::Ray ray;
	sm::vec3 cam_pos;

	auto cam_type = m_camera->TypeID();
	if (cam_type == pt0::GetCamTypeID<pt3::PerspCam>())
	{
		auto p_cam = std::dynamic_pointer_cast<pt3::PerspCam>(m_camera);
		ray.dir = m_vp.TransPos3ScreenToDir(
			sm::vec2(static_cast<float>(pos.x), static_cast<float>(pos.y)), *p_cam);
		ray.origin = p_cam->GetPos();
		cam_pos = p_cam->GetPos();
	}
	else if (cam_type == pt0::GetCamTypeID<pt3::OrthoCam>())
	{
		auto o_cam = std::dynamic_pointer_cast<pt3::OrthoCam>(m_camera);
		auto pos2 = o_cam->TransPosScreenToProject(pos.x, pos.y);
		switch (o_cam->GetViewPlaneType())
		{
		case pt3::OrthoCam::VP_ZY:
			ray.origin.Set(0, pos2.y, pos2.x);
			ray.dir.Set(-1, 0, 0);
			break;
		case pt3::OrthoCam::VP_XZ:
			ray.origin.Set(pos2.x, 0, pos2.y);
			ray.dir.Set(0, -1, 0);
			break;
		case pt3::OrthoCam::VP_XY:
			ray.origin.Set(pos2.x, pos2.y, 0);
			ray.dir.Set(0, 0, -1);
			break;
		}
		cam_pos = ray.origin;
	}

	m_stage.Traverse([&](const ee0::GameObj& obj)->bool
	{
		bool find = MeshPointQuery::Query(obj, ray, cam_pos, selected);
		if (find) {
			selected_obj = obj;
		}
		return !find;
	});

	if (GAME_OBJ_VALID(selected_obj))
	{
//		m_selected = selected;

		// insert to selection set
		std::vector<n0::NodeWithPos> nwps;
		nwps.push_back(n0::NodeWithPos(selected_obj, selected_obj, 0));
		ee0::MsgHelper::InsertSelection(*m_sub_mgr, nwps);

		UpdateCachedPolyBorder();
	}
	else
	{
		if (m_select_null) {
			ClearSelected();
		}
	}
}

void PolySelectOP::ClearSelected()
{
	m_selected.Reset();
	m_selected_poly.clear();
	m_selected_face.clear();
}

}
}