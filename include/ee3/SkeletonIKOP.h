#pragma once

#include <ee0/EditOP.h>
#include <ee0/typedef.h>

#include <SM_Plane.h>
#include <model/SkeletalAnim.h>

#include <memory>

namespace pt2 { class OrthoCamera; }
namespace pt3 { class Viewport; }
namespace model { class ModelInstance; }

namespace ee3
{

class SkeletonIKOP : public ee0::EditOP
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
	int QueryJointByPos(int x, int y) const;

	bool OneBone(int x, int y);
	bool TwoBones(int x, int y);

	sm::Plane GetRotatePlane(const sm::mat4& world_mat,
		const sm::vec3& pos, const sm::vec3& rot_axis = sm::vec3(0, 0, -1));
	static float GetRotateAngle(const sm::mat4& world_mat,
		const sm::vec3& base, const sm::vec3& from, const sm::vec3& to);

private:
	const pt3::Viewport& m_vp;
	ee0::SubjectMgrPtr   m_sub_mgr;

	std::shared_ptr<pt2::OrthoCamera> m_cam2d;

	model::ModelInstance* m_model = nullptr;

	int m_selecting = -1;
	int m_selected  = -1;

	bool m_active = false;

	// debug draw
	sm::vec3 m_debug[3];

}; // SkeletonIKOP

}