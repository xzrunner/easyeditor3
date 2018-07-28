#pragma once

#include "ee3/WorldTravelOP.h"
#include "ee3/MeshPointQuery.h"

#include <ee0/EditOP.h>
#include <ee0/typedef.h>
#include <ee0/SelectionSet.h>
#include <ee0/GameObj.h>

namespace ee0 { class WxStagePage; }
namespace pt3 { class Camera; class Viewport; }

namespace ee3
{

class PolySelectOP : public ee3::WorldTravelOP
{
public:
	PolySelectOP(ee0::WxStagePage& stage, pt3::Camera& cam,
		const pt3::Viewport& vp);

	virtual bool OnMouseLeftDown(int x, int y) override;

	virtual bool OnDraw() const override;

private:
	// for draw
	void CachePolyBorderPos();

private:
	ee0::WxStagePage&    m_stage;
	const pt3::Viewport& m_vp;

	const ee0::SelectionSet<ee0::GameObjWithPos>& m_selection;

	MeshPointQuery::Selected m_selected;

	std::vector<std::vector<sm::vec3>> m_selected_poly;

}; // PolySelectOP

}