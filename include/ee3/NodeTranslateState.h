#pragma once

#include <ee0/EditOpState.h>
#include <ee0/SelectionSet.h>
#include <ee0/typedef.h>

#include <node0/NodeWithPos.h>
#include <SM_Vector.h>

namespace pt3 { class Camera; class Viewport; }

namespace ee3
{

class NodeTranslateState : public ee0::EditOpState
{
public:
	NodeTranslateState(const pt3::Camera& cam, const pt3::Viewport& vp, 
		const ee0::SubjectMgrPtr& sub_mgr, const ee0::SelectionSet<n0::NodeWithPos>& selection);

	virtual bool OnMousePress(int x, int y) override;
	virtual bool OnMouseRelease(int x, int y) override;
	virtual bool OnMouseDrag(int x, int y) override;

private:
	void Translate(const sm::ivec2& first, const sm::ivec2& curr);

private:
	const pt3::Camera&   m_cam;
	const pt3::Viewport& m_vp;

	ee0::SubjectMgrPtr m_sub_mgr;

	const ee0::SelectionSet<n0::NodeWithPos>& m_selection;

	sm::ivec2 m_last_pos;

}; // NodeTranslateState

}
