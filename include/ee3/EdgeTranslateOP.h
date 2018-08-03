#pragma once

#include "ee3/MeshTranslateBaseOP.h"
#include "ee3/BrushEdge.h"

namespace ee3
{
namespace mesh
{

class EdgeTranslateOP : public MeshTranslateBaseOP<BrushEdge>
{
public:
	EdgeTranslateOP(const std::shared_ptr<pt0::Camera>& camera, const pt3::Viewport& vp,
		const ee0::SubjectMgrPtr& sub_mgr, const MeshPointQuery::Selected& selected,
		const ee0::SelectionSet<BrushEdge>& selection);

protected:
	virtual bool QueryByPos(const sm::vec2& pos, const BrushEdge& edge,
		const sm::mat4& cam_mat) const override;
	virtual void TranslateSelected(const sm::vec3& offset) override;

}; // EdgeTranslateOP

}
}