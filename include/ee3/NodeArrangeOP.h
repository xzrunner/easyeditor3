#pragma once

#include "ee3/NodeSelectOP.h"

#include <ee0/SelectionSet.h>
#include <ee0/typedef.h>
#include <ee0/GameObj.h>

namespace ee0 { class EditOpState; class WxStagePage; }
namespace pt3 { class Camera; class Viewport; }

namespace ee3
{

class WxStageCanvas;

class NodeArrangeOP : public NodeSelectOP
{
public:
	NodeArrangeOP(ee0::WxStagePage& stage, pt3::Camera& cam,
		const pt3::Viewport& vp);

	virtual bool OnKeyDown(int key_code) override;
	virtual bool OnMouseLeftDown(int x, int y) override;
	virtual bool OnMouseLeftUp(int x, int y) override;
	virtual bool OnMouseRightDown(int x, int y) override;
	virtual bool OnMouseRightUp(int x, int y) override;
	virtual bool OnMouseMove(int x, int y) override;
	virtual bool OnMouseDrag(int x, int y) override;
	virtual bool OnMouseWheelRotation(int x, int y, int direction) override;

private:
	void ChangeEditOpState(const ee0::EditOpStatePtr& state);

private:
	ee0::SubjectMgrPtr m_sub_mgr;

	ee0::SelectionSet<ee0::GameObjWithPos>& m_selection;

	std::shared_ptr<WxStageCanvas> m_canvas = nullptr;

	ee0::EditOpStatePtr m_op_state = nullptr;

	ee0::EditOpStatePtr m_cam_rotate_state    = nullptr;
	ee0::EditOpStatePtr m_cam_translate_state = nullptr;
	ee0::EditOpStatePtr m_cam_zoom_state      = nullptr;

	ee0::EditOpStatePtr m_node_rotate_state    = nullptr;
	ee0::EditOpStatePtr m_node_translate_state = nullptr;

	sm::ivec2 m_last_left_press;
	sm::ivec2 m_last_right_press;

}; // NodeArrangeOP

}
