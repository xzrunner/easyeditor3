#pragma once

#include <ee0/EditOpState.h>
#include <ee0/typedef.h>

namespace pt3 { class PerspCam; class Viewport; }

namespace ee3
{

class CamZoomState : public ee0::EditOpState
{
public:
	CamZoomState(pt3::PerspCam& cam, const pt3::Viewport& vp,
		const ee0::SubjectMgrPtr& sub_mgr);

	virtual bool OnMouseWheelRotation(int x, int y, int direction) override;

private:
	pt3::PerspCam&         m_cam;
	const pt3::Viewport& m_vp;
	ee0::SubjectMgrPtr   m_sub_mgr;

}; // CamZoomState

}