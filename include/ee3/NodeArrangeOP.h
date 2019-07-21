#pragma once

#include "ee3/NodeSelectOP.h"

#include <ee0/SelectionSet.h>
#include <ee0/typedef.h>
#include <ee0/GameObj.h>

namespace ee0 { class EditOpState; class WxStagePage; }
namespace pt3 { class Viewport; }

namespace ee3
{

class WxStageCanvas;

class NodeArrangeOP : public NodeSelectOP
{
public:
	NodeArrangeOP(const std::shared_ptr<pt0::Camera>& camera,
		ee0::WxStagePage& stage, const pt3::Viewport& vp);

	virtual bool OnKeyDown(int key_code) override;
	virtual bool OnMouseLeftDown(int x, int y) override;
	virtual bool OnMouseLeftUp(int x, int y) override;
    virtual bool OnMouseMiddleDown(int x, int y) override;
    virtual bool OnMouseMiddleUp(int x, int y) override;
	virtual bool OnMouseRightDown(int x, int y) override;
	virtual bool OnMouseRightUp(int x, int y) override;
	virtual bool OnMouseMove(int x, int y) override;
	virtual bool OnMouseDrag(int x, int y) override;
	virtual bool OnMouseWheelRotation(int x, int y, int direction) override;

    virtual bool OnDraw() const override;

protected:
    // callback func
    virtual void AfterInsertSelected(const n0::SceneNodePtr& node) const override;
    virtual void AfterDeleteSelected(const n0::SceneNodePtr& node) const override;
    virtual void AfterClearSelection() const override;

private:
	ee0::SubjectMgrPtr m_sub_mgr;

	ee0::SelectionSet<ee0::GameObjWithPos>& m_selection;

	std::shared_ptr<WxStageCanvas> m_canvas = nullptr;

	ee0::EditOpStatePtr m_cam_rotate_state    = nullptr;
	ee0::EditOpStatePtr m_cam_translate_state = nullptr;

	ee0::EditOpStatePtr m_node_translate_state = nullptr;

	sm::ivec2 m_last_left_press;
    sm::ivec2 m_last_middle_press;
	sm::ivec2 m_last_right_press;

}; // NodeArrangeOP

}
