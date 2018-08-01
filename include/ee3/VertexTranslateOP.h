#pragma once

#include "ee3/MeshPointQuery.h"

#include <ee0/EditOP.h>
#include <ee0/typedef.h>

#include <quake/MapModel.h>
#include <painting2/OrthoCamera.h>

namespace pt3 { class ICamera; class Viewport; }

namespace ee3
{

class VertexTranslateOP : public ee0::EditOP
{
public:
	VertexTranslateOP(pt3::ICamera& cam, const pt3::Viewport& vp,
		const ee0::SubjectMgrPtr& sub_mgr, const MeshPointQuery::Selected& selected);

	virtual bool OnKeyDown(int key_code) override;
	virtual bool OnKeyUp(int key_code) override;

	virtual bool OnMouseLeftDown(int x, int y) override;
	virtual bool OnMouseLeftUp(int x, int y) override;
	virtual bool OnMouseMove(int x, int y) override;
	virtual bool OnMouseDrag(int x, int y) override;

	virtual bool OnDraw() const override;

private:
	quake::BrushVertexPtr QueryByPos(int x, int y) const;

private:
	const pt3::ICamera&   m_cam;
	const pt3::Viewport& m_vp;
	ee0::SubjectMgrPtr   m_sub_mgr;

	pt2::OrthoCamera m_cam2d;

	const MeshPointQuery::Selected& m_selected;

	quake::BrushVertexPtr m_selecting_vert = nullptr;
	quake::BrushVertexPtr m_selected_vert  = nullptr;

}; // VertexTranslateOP

}