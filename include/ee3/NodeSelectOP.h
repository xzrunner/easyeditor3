#pragma once

#include <ee0/NodeSelectOP.h>

namespace ee0 { class WxStagePage; }

namespace ee3
{

class NodeSelectOP : public ee0::NodeSelectOP
{
public:	
	NodeSelectOP(ee0::WxStagePage& stage);

	virtual bool OnDraw() const override;

private:
	virtual n0::SceneNodePtr QueryByPos(int screen_x, int screen_y) const override;
	virtual void QueryByRect(const sm::ivec2& p0, const sm::ivec2& p1,
		bool contain, std::vector<n0::SceneNodePtr>& result) const {}

}; // NodeSelectOP

}
