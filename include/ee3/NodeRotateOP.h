#pragma once

#include <ee0/EditOP.h>
#include <ee0/typedef.h>

namespace ee0 { class WxStagePage; }
namespace pt3 { class Camera; class Viewport; }

namespace ee3
{

class NodeRotateOP : public ee0::EditOP
{
public:
	NodeRotateOP(ee0::WxStagePage& stage, pt3::Camera& cam,
		const pt3::Viewport& vp);

	virtual bool OnMouseLeftDown(int x, int y) override;
	virtual bool OnMouseLeftUp(int x, int y) override;
	virtual bool OnMouseRightDown(int x, int y);
	virtual bool OnMouseRightUp(int x, int y);
	virtual bool OnMouseDrag(int x, int y);
	virtual bool OnMouseWheelRotation(int x, int y, int direction) override;

	virtual bool OnActive() override;
	virtual bool OnDraw() const override;

private:
	void ChangeEditOpState(const ee0::EditOpStatePtr& state);

private:
	ee0::SubjectMgrPtr m_sub_mgr;

	ee0::EditOpStatePtr m_op_state = nullptr;

	ee0::EditOpStatePtr m_cam_rotate_state    = nullptr;
	ee0::EditOpStatePtr m_cam_translate_state = nullptr;
	ee0::EditOpStatePtr m_cam_zoom_state      = nullptr;

	ee0::EditOpStatePtr m_node_rotate_state   = nullptr;

}; // NodeRotateOP

}