#pragma once

#include <ee0/EditOpState.h>
#include <ee0/typedef.h>

#include <SM_Vector.h>

namespace pt3 { class Camera; }

namespace ee3
{

class CamTranslateState : public ee0::EditOpState
{
public:
	CamTranslateState(pt3::Camera& cam, const ee0::SubjectMgrPtr& sub_mgr);

	virtual bool OnMousePress(int x, int y) override;
	virtual bool OnMouseRelease(int x, int y) override;
	virtual bool OnMouseDrag(int x, int y) override;

private:
	pt3::Camera&       m_cam;
	ee0::SubjectMgrPtr m_sub_mgr;

	sm::ivec2 m_last_pos;

}; // CamTranslateState

}