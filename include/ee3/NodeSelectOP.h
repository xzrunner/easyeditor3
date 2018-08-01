#pragma once

#include <ee0/NodeSelectOP.h>

namespace ee0 { class WxStagePage; }
namespace pt3 { class PerspCam; class Viewport; }

namespace ee3
{

class NodeSelectOP : public ee0::NodeSelectOP
{
public:	
	NodeSelectOP(ee0::WxStagePage& stage, pt3::PerspCam& cam,
		const pt3::Viewport& vp);

	virtual bool OnDraw() const override;

private:
	virtual ee0::GameObj QueryByPos(int screen_x, int screen_y) const override;
	virtual void QueryByRect(const sm::ivec2& p0, const sm::ivec2& p1,
		bool contain, std::vector<ee0::GameObj>& result) const {}

private:
	pt3::PerspCam&       m_cam;
	const pt3::Viewport& m_vp;

}; // NodeSelectOP

}
