#pragma once

#include <ee0/typedef.h>

#include <memory>

namespace model { class ModelInstance; }
namespace pt0 { class Camera; }
namespace pt2 { class OrthoCamera; }
namespace pt3 { class Viewport; }

namespace ee3
{

class SkeletonOpImpl
{
public:
	SkeletonOpImpl(const pt3::Viewport& vp,
		const ee0::SubjectMgrPtr& sub_mgr);

	int QueryJointByPos(const pt0::Camera& cam,
		int x, int y) const;

	void OnDraw() const;

protected:
	const pt3::Viewport& m_vp;
	ee0::SubjectMgrPtr   m_sub_mgr;

	std::shared_ptr<pt2::OrthoCamera> m_cam2d;

	model::ModelInstance* m_model = nullptr;

	int m_selecting = -1;
	int m_selected = -1;

}; // SkeletonOpImpl

}