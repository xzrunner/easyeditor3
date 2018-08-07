#pragma once

#include <ee3/CameraMoveOP.h>

#include <SM_Vector.h>

namespace ee3
{

class CameraFlyOP : public CameraMoveOP
{
public:
	CameraFlyOP(const std::shared_ptr<pt0::Camera>& camera,
		const ee0::SubjectMgrPtr& sub_mgr);

	virtual bool OnMouseMove(int x, int y);

private:
	ee0::SubjectMgrPtr m_sub_mgr;

	sm::ivec2 m_last_pos;

}; // CameraFlyOP

}