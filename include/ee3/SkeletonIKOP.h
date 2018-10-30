#pragma once

#include "ee3/SkeletonSelectOp.h"

#include <SM_Plane.h>
#include <SM_Matrix.h>

#include <memory>
#include <vector>

namespace ee3
{

class SkeletonIKOP : public SkeletonSelectOp
{
public:
	SkeletonIKOP(const std::shared_ptr<pt0::Camera>& camera,
		const pt3::Viewport& vp, const ee0::SubjectMgrPtr& sub_mgr);

	virtual bool OnMouseLeftDown(int x, int y) override;
	virtual bool OnMouseLeftUp(int x, int y) override;
	virtual bool OnMouseMove(int x, int y) override;
	virtual bool OnMouseDrag(int x, int y) override;

	virtual bool OnDraw() const override;

private:
	bool OneBone(int x, int y);
	bool TwoBones(int x, int y);

	sm::Plane GetRotatePlane(const sm::mat4& world_mat,
		const sm::vec3& pos, const sm::vec3& rot_axis);
	static float GetRotateAngle(const sm::mat4& world_mat,
		const sm::vec3& base, const sm::vec3& from, const sm::vec3& to);

private:
	bool m_active = false;

}; // SkeletonIKOP

}