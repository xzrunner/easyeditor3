#pragma once

#include <ee0/EditOpState.h>
#include <ee0/typedef.h>

namespace pt3 { class Viewport; }

namespace ee3
{

class CamZoomState : public ee0::EditOpState
{
public:
	CamZoomState(const std::shared_ptr<pt0::Camera>& camera,
		const pt3::Viewport& vp, const ee0::SubjectMgrPtr& sub_mgr);

	virtual bool OnMouseWheelRotation(int x, int y, int direction) override;

private:
	const pt3::Viewport& m_vp;
	ee0::SubjectMgrPtr   m_sub_mgr;

}; // CamZoomState

}