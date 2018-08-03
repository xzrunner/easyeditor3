#pragma once

#include <ee0/EditOpState.h>
#include <ee0/SelectionSet.h>
#include <ee0/typedef.h>
#include <ee0/GameObj.h>

#include <SM_Vector.h>

namespace pt3 { class Viewport; }

namespace ee3
{

class NodeRotateState : public ee0::EditOpState
{
public:
	NodeRotateState(const std::shared_ptr<pt0::Camera>& camera, const pt3::Viewport& vp,
		const ee0::SubjectMgrPtr& sub_mgr, const ee0::SelectionSet<ee0::GameObjWithPos>& selection);

	virtual bool OnMousePress(int x, int y) override;
	virtual bool OnMouseRelease(int x, int y) override;
	virtual bool OnMouseDrag(int x, int y) override;

private:
	void Rotate(const sm::ivec2& start, const sm::ivec2& end);

	sm::vec2 TransPos3ProjectToScreen(const sm::vec3& proj) const;

private:
	const pt3::Viewport& m_vp;

	ee0::SubjectMgrPtr m_sub_mgr;

	const ee0::SelectionSet<ee0::GameObjWithPos>& m_selection;

	sm::ivec2 m_last_pos;

}; // NodeRotateState

}