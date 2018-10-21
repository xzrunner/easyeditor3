#pragma once

#include <ee0/NodeSelectOP.h>

namespace ee0 { class WxStagePage; }
namespace pt3 { class Viewport; }

namespace ee3
{

class NodeSelectOP : public ee0::NodeSelectOP
{
public:
	NodeSelectOP(const std::shared_ptr<pt0::Camera>& camera,
		ee0::WxStagePage& stage, const pt3::Viewport& vp);

	virtual bool OnDraw() const override;

private:
	virtual ee0::GameObj QueryByPos(int screen_x, int screen_y) const override;
	virtual void QueryByRect(const sm::ivec2& p0, const sm::ivec2& p1,
		bool contain, std::vector<ee0::GameObj>& result) const {}

private:
	const pt3::Viewport& m_vp;

}; // NodeSelectOP

}
