#pragma once

#include "ee3/MeshTranslateBaseOP.h"

namespace ee3
{
namespace mesh
{

class VertexTranslateOP : public MeshTranslateBaseOP<quake::BrushVertexPtr>
{
public:
	VertexTranslateOP(const std::shared_ptr<pt0::Camera>& camera, const pt3::Viewport& vp,
		const ee0::SubjectMgrPtr& sub_mgr, const MeshPointQuery::Selected& selected,
		const ee0::SelectionSet<quake::BrushVertexPtr>& selection, std::function<void()> update_cb);

protected:
	virtual bool QueryByPos(const sm::vec2& pos, const quake::BrushVertexPtr& vert,
		const sm::mat4& cam_mat) const override;
	virtual void TranslateSelected(const sm::vec3& offset) override;

}; // VertexTranslateOP

}
}