#include "ee3/MeshFaceOP.h"
#include "ee3/FacePushPullState.h"

#include <ee0/WxStagePage.h>
#include <ee0/SubjectMgr.h>
#include <ee0/color_config.h>
#include <ee0/EditOpState.h>

#include <painting2/Color.h>
#include <painting3/PrimitiveDraw.h>
#include <painting3/PerspCam.h>
#include <painting3/Viewport.h>

namespace ee3
{

MeshFaceOP::MeshFaceOP(ee0::WxStagePage& stage, pt3::PerspCam& cam,
	                   const pt3::Viewport& vp)
	: WorldTravelOP(cam, vp, stage.GetSubjectMgr())
	, m_vp(vp)
	, m_selection(stage.GetSelection())
{
}

bool MeshFaceOP::OnKeyDown(int key_code)
{
	if (ee0::EditOP::OnKeyDown(key_code)) { return true; }

	//if (key_code == WXK_SHIFT && m_selected.face)
	//{
	//	m_last_st = m_op_state;
	//	auto pushpull = std::make_shared<FacePushPullState>(
	//		m_cam, m_vp, m_sub_mgr, m_selected.face, m_selected.mat);
	//	ChangeEditOpState(pushpull);

	//	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
	//}

	return false;
}

bool MeshFaceOP::OnKeyUp(int key_code)
{
	if (ee0::EditOP::OnKeyUp(key_code)) { return true; }

	if (key_code == WXK_SHIFT) {
		ChangeEditOpState(m_last_st);
	}

	return false;
}

bool MeshFaceOP::OnMouseLeftDown(int x, int y)
{
	sm::vec3 ray_dir = m_vp.TransPos3ScreenToDir(
		sm::vec2(static_cast<float>(x), static_cast<float>(y)), m_cam);
	sm::Ray ray(m_cam.GetPos(), ray_dir);

	//bool find = MeshPointQuery::Query(m_selection, ray, m_cam.GetPos(), m_selected);
	//if (!find && WorldTravelOP::OnMouseLeftDown(x, y)) {
	//	return true;
	//}

	if (m_op_state->OnMousePress(x, y)) {
		return true;
	}

	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

	return false;
}

bool MeshFaceOP::OnDraw() const
{
	if (WorldTravelOP::OnDraw()) {
		return true;
	}

	if (!m_selected.poly || !m_selected.face) {
		return false;
	}

	pt3::PrimitiveDraw::SetColor(ee0::LIGHT_RED.ToABGR());
	std::vector<sm::vec3> polyline;
	m_selected.face->GetBorder(polyline);
	for (int i = 0, n = polyline.size(); i < n; ++i) {
		polyline[i] = m_selected.mat * polyline[i];
	}
	pt3::PrimitiveDraw::Polygon(polyline);

	if (m_selected.pos.IsValid())
	{
		pt3::PrimitiveDraw::SetColor(ee0::LIGHT_GREEN.ToABGR());
		pt3::PrimitiveDraw::Line(m_selected.pos, m_selected.pos + m_selected.mat * m_selected.normal * 100000);
	}

	// todo
	m_op_state->OnDraw();

	return false;
}

}