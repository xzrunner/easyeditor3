#pragma once

#include <ee0/EditOP.h>
#include <ee0/typedef.h>

#include <SM_Vector.h>

namespace pt3 { class Camera; class Viewport; }

namespace ee3
{

class CameraMoveOP : public ee0::EditOP
{
public:
	CameraMoveOP(pt3::Camera& cam, const pt3::Viewport& vp,
		const ee0::SubjectMgrPtr& sub_mgr);

	virtual bool OnKeyDown(int key_code) override;
	virtual bool OnKeyUp(int key_code) override;
	virtual bool OnMouseRightDown(int x, int y) override;
	virtual bool OnMouseRightUp(int x, int y) override;
	virtual bool OnMouseDrag(int x, int y) override;
	virtual bool OnMouseWheelRotation(int x, int y, int direction) override;

	virtual bool Update(float dt) override;

private:
	enum MoveDir
	{
		MOVE_NONE = 0,
		MOVE_LEFT,
		MOVE_RIGHT,
		MOVE_UP,
		MOVE_DOWN,
	};

private:
	pt3::Camera&         m_cam;
	const pt3::Viewport& m_vp;
	ee0::SubjectMgrPtr   m_sub_mgr;

	sm::ivec2 m_last_pos;

	MoveDir m_move_dir = MOVE_NONE;

}; // CameraMoveOP

}