#pragma once

#include "ee3/CameraMoveOP.h"

#include <ee0/EditOP.h>
#include <ee0/typedef.h>

#include <SM_Vector.h>

namespace pt3 { class Viewport; }

namespace ee3
{

class CameraDriveOP : public CameraMoveOP
{
public:
	CameraDriveOP(const std::shared_ptr<pt0::Camera>& camera,
		const pt3::Viewport& vp, const ee0::SubjectMgrPtr& sub_mgr);

	virtual bool OnMouseRightDown(int x, int y) override;
	virtual bool OnMouseRightUp(int x, int y) override;
	virtual bool OnMouseDrag(int x, int y) override;
	virtual bool OnMouseWheelRotation(int x, int y, int direction) override;

private:
	const pt3::Viewport& m_vp;
	ee0::SubjectMgrPtr   m_sub_mgr;

	sm::ivec2 m_last_pos;

}; // CameraDriveOP

}