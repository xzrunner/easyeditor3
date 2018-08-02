#pragma once

#include "ee3/MeshPointQuery.h"

#include <ee0/EditOP.h>
#include <ee0/typedef.h>

#include <SM_Rect.h>
#include <quake/MapModel.h>
#include <painting2/OrthoCamera.h>

#include <set>

namespace pt3 { class ICamera; class Viewport; }

namespace ee3
{

class VertexSelectOP : public ee0::EditOP
{
public:
	VertexSelectOP(pt3::ICamera& cam, const pt3::Viewport& vp,
		const ee0::SubjectMgrPtr& sub_mgr, const MeshPointQuery::Selected& selected);

	virtual bool OnMouseLeftDown(int x, int y) override;
	virtual bool OnMouseLeftUp(int x, int y) override;
	virtual bool OnMouseMove(int x, int y) override;
	virtual bool OnMouseDrag(int x, int y) override;

	virtual bool OnDraw() const override;

private:
	quake::BrushVertexPtr QueryByPos(int x, int y) const;
	void QueryByRect(const sm::irect& rect, std::vector<quake::BrushVertexPtr>& selection) const;

private:
	const pt3::ICamera&  m_cam;
	const pt3::Viewport& m_vp;
	ee0::SubjectMgrPtr   m_sub_mgr;

	const MeshPointQuery::Selected& m_base_selected;

	pt2::OrthoCamera m_cam2d;

	ee0::SelectionSet<quake::BrushVertexPtr> m_selected;
	quake::BrushVertexPtr                    m_selecting = nullptr;

	sm::ivec2 m_first_pos;

	std::unique_ptr<ee0::EditOpState> m_draw_state = nullptr;

}; // VertexSelectOP

}