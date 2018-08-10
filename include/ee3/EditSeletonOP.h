#pragma once

#include <ee0/EditOP.h>

#include <model/SkeletalAnim.h>

#include <memory>

namespace ee0 { class WxStagePage; }
namespace pt3 { class Viewport; }
namespace model { class ModelInstance; }

namespace ee3
{

class EditSeletonOP : public ee0::EditOP
{
public:
	EditSeletonOP(const std::shared_ptr<pt0::Camera>& camera,
		ee0::WxStagePage& stage, const pt3::Viewport& vp);

	virtual bool OnMouseLeftDown(int x, int y) override;
	virtual bool OnMouseLeftUp(int x, int y) override;
	virtual bool OnMouseMove(int x, int y) override;
	virtual bool OnMouseDrag(int x, int y) override;

	virtual bool OnDraw() const;

	void SetModel(model::ModelInstance* model) {
		m_model = model;
	}

private:
	model::ModelInstance* m_model = nullptr;

}; // EditSeletonOP

}