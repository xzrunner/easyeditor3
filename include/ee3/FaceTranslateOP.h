#pragma once

#include "ee3/MeshTranslateBaseOP.h"

namespace ee3
{
namespace mesh
{

class FaceTranslateOP : public MeshTranslateBaseOP<quake::BrushFacePtr>
{
public:
	FaceTranslateOP(pt3::PerspCam& cam, const pt3::Viewport& vp,
		const ee0::SubjectMgrPtr& sub_mgr, const MeshPointQuery::Selected& selected,
		const ee0::SelectionSet<quake::BrushFacePtr>& selection);

protected:
	virtual bool QueryByPos(const sm::vec2& pos, const quake::BrushFacePtr& face,
		const sm::mat4& cam_mat) const override;
	virtual void TranslateSelected(const sm::vec3& offset) override;

}; // FaceTranslateOP

}
}