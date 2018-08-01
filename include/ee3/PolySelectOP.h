#pragma once

#include "ee3/MeshPointQuery.h"

#include <ee0/EditOP.h>
#include <ee0/typedef.h>
#include <ee0/SelectionSet.h>
#include <ee0/GameObj.h>

namespace ee0 { class WxStagePage; }
namespace pt3 { class PerspCam; class Viewport; }

namespace ee3
{

class PolySelectOP : public ee0::EditOP
{
public:
	PolySelectOP(ee0::WxStagePage& stage, pt3::PerspCam& cam,
		const pt3::Viewport& vp);

	virtual bool OnKeyDown(int key_code) override;
	virtual bool OnKeyUp(int key_code) override;

	virtual bool OnMouseLeftDown(int x, int y) override;
	virtual bool OnMouseMove(int x, int y) override;
	virtual bool OnMouseDrag(int x, int y) override;

	virtual bool OnDraw() const override;

	auto& GetSelected() const { return m_selected; }

	void SetCanSelectNull(bool select_null) {
		m_select_null = select_null;
	}

private:
	// for draw
	void UpdatePolyBorderPos();

private:
	ee0::WxStagePage&    m_stage;

	pt3::PerspCam&       m_cam;
	const pt3::Viewport& m_vp;
	ee0::SubjectMgrPtr   m_sub_mgr;

	const ee0::SelectionSet<ee0::GameObjWithPos>& m_selection;

	MeshPointQuery::Selected m_selected;

	// cache for draw
	std::vector<std::vector<sm::vec3>> m_selected_poly;
	std::vector<sm::vec3>              m_selected_face;

	bool m_move_select = false;

	bool m_select_null = true;

}; // PolySelectOP

}