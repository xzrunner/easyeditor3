#pragma once

#include <ee0/EditOP.h>
#include <ee0/typedef.h>

namespace pt3 { class PerspCam; class Viewport; }

namespace ee3
{

class PerspCam;

class WorldTravelOP : public ee0::EditOP
{
public:
	WorldTravelOP(pt3::PerspCam& cam, const pt3::Viewport& vp,
		const ee0::SubjectMgrPtr& sub_mgr);

	virtual bool OnKeyDown(int key_code) override;
	virtual bool OnMouseLeftDown(int x, int y) override;
	virtual bool OnMouseLeftUp(int x, int y) override;
	virtual bool OnMouseRightDown(int x, int y) override;
	virtual bool OnMouseRightUp(int x, int y) override;
	virtual bool OnMouseDrag(int x, int y) override;
	virtual bool OnMouseMove(int x, int y) override;
	virtual bool OnMouseWheelRotation(int x, int y, int direction) override;

protected:
	void ChangeEditOpState(const ee0::EditOpStatePtr& state);

protected:
	pt3::PerspCam&        m_cam;
	ee0::SubjectMgrPtr  m_sub_mgr;

	ee0::EditOpStatePtr m_op_state = nullptr;

	ee0::EditOpStatePtr m_rotate_state = nullptr;
	ee0::EditOpStatePtr m_translate_state = nullptr;
	ee0::EditOpStatePtr m_zoom_state = nullptr;

}; // WorldTravelOP

}