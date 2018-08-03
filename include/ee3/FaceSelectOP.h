#pragma once

#include "ee3/MeshSelectBaseOP.h"

namespace ee3
{
namespace mesh
{

class FaceSelectOP : public MeshSelectBaseOP<quake::BrushFacePtr>
{
public:
	FaceSelectOP(const std::shared_ptr<pt0::Camera>& camera, const pt3::Viewport& vp,
		const ee0::SubjectMgrPtr& sub_mgr, const MeshPointQuery::Selected& selected);

protected:
	virtual void DrawImpl(const quake::MapBrush& brush, const sm::mat4& cam_mat) const override;

	virtual quake::BrushFacePtr QueryByPos(int x, int y) const override;
	virtual void QueryByRect(const sm::irect& rect, std::vector<quake::BrushFacePtr>& selection) const override;

private:
	sm::vec2 CalcFaceCenter(const quake::BrushFace& face, const sm::mat4& cam_mat) const;

	void DrawFace(const quake::BrushFace& face, uint32_t color, const sm::mat4& cam_mat) const;

}; // FaceSelectOP

}
}