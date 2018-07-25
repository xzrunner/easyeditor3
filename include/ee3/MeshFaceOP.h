#pragma once

#include "ee3/WorldTravelOP.h"

#include <ee0/typedef.h>
#include <ee0/SelectionSet.h>
#include <ee0/GameObj.h>

#include <SM_Matrix.h>
#include <halfedge/Polyhedron.h>

namespace ee0 { class WxStagePage; }
namespace pt3 { class Camera; class Viewport; }
namespace he { struct Face; }

namespace ee3
{

class MeshFaceOP : public ee3::WorldTravelOP
{
public:
	MeshFaceOP(ee0::WxStagePage& stage, pt3::Camera& cam,
		const pt3::Viewport& vp);

	virtual bool OnMouseLeftDown(int x, int y) override;

	virtual bool OnDraw() const override;

private:
	void PointQuery(int x, int y);

private:
	const pt3::Viewport& m_vp;

	const ee0::SelectionSet<ee0::GameObjWithPos>& m_selection;

	sm::mat4 m_selected_mat;
	he::PolyhedronPtr m_selected_poly = nullptr;
	he::FacePtr       m_selected_face = nullptr;

}; // MeshFaceOP

}