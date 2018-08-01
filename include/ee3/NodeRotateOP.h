#pragma once

#include <ee0/EditOP.h>
#include <ee0/typedef.h>

namespace ee0 { class WxStagePage; }
namespace pt3 { class PerspCam; class Viewport; }

namespace ee3
{

class NodeRotateOP : public ee0::EditOP
{
public:
	NodeRotateOP(ee0::WxStagePage& stage, pt3::PerspCam& cam,
		const pt3::Viewport& vp);

	virtual bool OnMouseLeftDown(int x, int y) override;
	virtual bool OnMouseLeftUp(int x, int y) override;
	virtual bool OnMouseDrag(int x, int y);

	virtual bool OnActive() override;
	virtual bool OnDraw() const override;

private:
	ee0::SubjectMgrPtr m_sub_mgr;

	ee0::EditOpStatePtr m_rotate_state = nullptr;

}; // NodeRotateOP

}