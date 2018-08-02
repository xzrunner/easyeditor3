#pragma once

#include "ee3/MeshPointQuery.h"

#include <ee0/EditOP.h>
#include <ee0/typedef.h>
#include <ee0/SelectionSet.h>

#include <quake/MapModel.h>
#include <painting2/OrthoCamera.h>

namespace pt3 { class PerspCam; class Viewport; }

namespace ee3
{
namespace mesh
{

class VertexTranslateOP : public ee0::EditOP
{
public:
	VertexTranslateOP(pt3::PerspCam& cam, const pt3::Viewport& vp,
		const ee0::SubjectMgrPtr& sub_mgr, const MeshPointQuery::Selected& selected,
		const ee0::SelectionSet<quake::BrushVertexPtr>& selection);

	virtual bool OnMouseLeftDown(int x, int y) override;
	virtual bool OnMouseLeftUp(int x, int y) override;
	virtual bool OnMouseDrag(int x, int y) override;

private:
	void TranslateSelected(const sm::vec3& offset);

private:
	const pt3::PerspCam& m_cam;
	const pt3::Viewport& m_vp;

	ee0::SubjectMgrPtr   m_sub_mgr;

	const MeshPointQuery::Selected&                 m_selected;
	const ee0::SelectionSet<quake::BrushVertexPtr>& m_selection;

	pt2::OrthoCamera m_cam2d;

	sm::vec3 m_last_pos;

}; // VertexTranslateOP

}
}