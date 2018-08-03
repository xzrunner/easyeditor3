#pragma once

#include <ee0/EditOP.h>
#include <ee0/typedef.h>

namespace pt3 { class Viewport; }

namespace ee3
{

class WorldTravelOP : public ee0::EditOP
{
public:
	WorldTravelOP(const std::shared_ptr<pt0::Camera>& camera, 
		const pt3::Viewport& vp, const ee0::SubjectMgrPtr& sub_mgr);

	virtual bool OnKeyDown(int key_code) override;
	virtual bool OnMouseLeftDown(int x, int y) override;
	virtual bool OnMouseLeftUp(int x, int y) override;
	virtual bool OnMouseRightDown(int x, int y) override;
	virtual bool OnMouseRightUp(int x, int y) override;
	virtual bool OnMouseDrag(int x, int y) override;
	virtual bool OnMouseMove(int x, int y) override;
	virtual bool OnMouseWheelRotation(int x, int y, int direction) override;

protected:
	ee0::SubjectMgrPtr  m_sub_mgr;

	ee0::EditOpStatePtr m_rotate_state = nullptr;
	ee0::EditOpStatePtr m_translate_state = nullptr;
	ee0::EditOpStatePtr m_zoom_state = nullptr;

}; // WorldTravelOP

}