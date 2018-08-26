#pragma once

#include "ee3/SkeletonSelectOp.h"

#include <ee0/EditOP.h>
#include <ee0/typedef.h>

#include <memory>

namespace ee3
{

class SkeletonJointOP : public SkeletonSelectOp
{
public:
	SkeletonJointOP(const std::shared_ptr<pt0::Camera>& camera,
		const pt3::Viewport& vp, const ee0::SubjectMgrPtr& sub_mgr);

	virtual bool OnMouseLeftDown(int x, int y) override;
	virtual bool OnMouseLeftUp(int x, int y) override;
	virtual bool OnMouseMove(int x, int y) override;
	virtual bool OnMouseDrag(int x, int y) override;

	virtual bool OnActive() override;
	virtual bool OnDraw() const override;



	void ChangeToOpRotate();
	void ChangeToOpTranslate();

private:
	void InitRotateState();
	void InitTranslateState();

private:
	ee0::EditOpStatePtr m_rotate_state    = nullptr;
	ee0::EditOpStatePtr m_translate_state = nullptr;

}; // SkeletonJointOP

}