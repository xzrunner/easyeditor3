#pragma once

#include <ee0/typedef.h>
#include <ee0/EditOP.h>

#include <memory>

namespace ur { class Device; class Context; }
namespace model { class ModelInstance; }
namespace pt0 { class Camera; }
namespace pt2 { class OrthoCamera; }
namespace pt3 { class Viewport; }

namespace ee3
{

class SkeletonSelectOp : public ee0::EditOP
{
public:
	SkeletonSelectOp(const ur::Device& dev, ur::Context& ctx,
        const std::shared_ptr<pt0::Camera>& camera,
		const pt3::Viewport& vp, const ee0::SubjectMgrPtr& sub_mgr);

	virtual bool OnDraw(const ur::Device& dev, ur::Context& ctx) const override;

	void SetModel(model::ModelInstance* model) {
		m_model = model;
	}
	const model::ModelInstance* GetModel() const {
		return m_model;
	}

	void SetSelected(int selected);
	int GetSelected() const { return m_selected; }

protected:
	int QueryJointByPos(const pt0::Camera& cam,
		int x, int y) const;

protected:
    const ur::Device& m_dev;
    ur::Context& m_ctx;

	const pt3::Viewport& m_vp;
	ee0::SubjectMgrPtr   m_sub_mgr;

	std::shared_ptr<pt2::OrthoCamera> m_cam2d;

	model::ModelInstance* m_model = nullptr;

	int m_selecting = -1;
	int m_selected = -1;

}; // SkeletonSelectOp

}