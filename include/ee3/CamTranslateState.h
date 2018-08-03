#pragma once

#include <ee0/EditOpState.h>
#include <ee0/typedef.h>

#include <SM_Vector.h>

namespace ee3
{

class CamTranslateState : public ee0::EditOpState
{
public:
	CamTranslateState(const std::shared_ptr<pt0::Camera>& camera, 
		const ee0::SubjectMgrPtr& sub_mgr);

	virtual bool OnMousePress(int x, int y) override;
	virtual bool OnMouseRelease(int x, int y) override;
	virtual bool OnMouseDrag(int x, int y) override;

private:
	ee0::SubjectMgrPtr m_sub_mgr;

	sm::ivec2 m_last_pos;

}; // CamTranslateState

}