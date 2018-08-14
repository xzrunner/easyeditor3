#pragma once

#include "ee3/SkeletonOpImpl.h"

#include <ee0/EditOP.h>
#include <ee0/typedef.h>

#include <SM_Plane.h>
#include <SM_Matrix.h>

#include <memory>

namespace ee3
{

class SkeletonIKOP : public ee0::EditOP, private SkeletonOpImpl
{
public:
	SkeletonIKOP(const std::shared_ptr<pt0::Camera>& camera,
		const pt3::Viewport& vp, const ee0::SubjectMgrPtr& sub_mgr);

	virtual bool OnMouseLeftDown(int x, int y) override;
	virtual bool OnMouseLeftUp(int x, int y) override;
	virtual bool OnMouseMove(int x, int y) override;
	virtual bool OnMouseDrag(int x, int y) override;

	virtual bool OnDraw() const override;

	void SetModel(model::ModelInstance* model) {
		m_model = model;
	}

private:
	bool OneBone(int x, int y);
	bool TwoBones(int x, int y);

	sm::Plane GetRotatePlane(const sm::mat4& world_mat,
		const sm::vec3& pos, const sm::vec3& rot_axis = sm::vec3(0, 0, -1));
	static float GetRotateAngle(const sm::mat4& world_mat,
		const sm::vec3& base, const sm::vec3& from, const sm::vec3& to);

private:
	bool m_active = false;

	// debug draw
	sm::vec3 m_debug[3];

}; // SkeletonIKOP

}