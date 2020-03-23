#pragma once

#include <ee0/EditOP.h>
#include <ee0/typedef.h>

namespace ee3
{

class CameraMoveOP : public ee0::EditOP
{
public:
	CameraMoveOP(const std::shared_ptr<pt0::Camera>& camera,
		const ee0::SubjectMgrPtr& sub_mgr);

	virtual bool OnKeyDown(int key_code) override;
	virtual bool OnKeyUp(int key_code) override;

	virtual bool Update(float dt) override;

private:
	enum MoveDir
	{
		MOVE_NONE = 0,
		MOVE_LEFT,
		MOVE_RIGHT,
		MOVE_UP,
		MOVE_DOWN,
        MOVE_NEAR,
        MOVE_FAR,
	};

private:
	ee0::SubjectMgrPtr m_sub_mgr;

	MoveDir m_move_dir = MOVE_NONE;

}; // CameraMoveOP

}