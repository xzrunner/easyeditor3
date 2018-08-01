#pragma once

#include "ee3/WorldTravelOP.h"
#include "ee3/MeshPointQuery.h"

#include <ee0/typedef.h>
#include <ee0/SelectionSet.h>
#include <ee0/GameObj.h>

#include <SM_Matrix.h>
#include <halfedge/Polyhedron.h>

namespace ee0 { class WxStagePage; }
namespace pt3 { class PerspCam; class Viewport; }
namespace he { struct Face; }

namespace ee3
{

class MeshFaceOP : public ee3::WorldTravelOP
{
public:
	MeshFaceOP(ee0::WxStagePage& stage, pt3::PerspCam& cam,
		const pt3::Viewport& vp);

	virtual bool OnKeyDown(int key_code) override;
	virtual bool OnKeyUp(int key_code) override;
	virtual bool OnMouseLeftDown(int x, int y) override;

	virtual bool OnDraw() const override;

private:
	const pt3::Viewport& m_vp;

	const ee0::SelectionSet<ee0::GameObjWithPos>& m_selection;

	ee0::EditOpStatePtr m_last_st = nullptr;

	MeshPointQuery::Selected m_selected;

}; // MeshFaceOP

}