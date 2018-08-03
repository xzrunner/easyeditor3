#pragma once

#include <ee0/EditOpState.h>
#include <ee0/SelectionSet.h>
#include <ee0/typedef.h>
#include <ee0/GameObj.h>

#include <SM_Vector.h>
#include <painting0/Camera.h>
#include <painting2/OrthoCamera.h>

namespace pt3 { class Viewport; }

namespace ee3
{

class NodeRotate3State : public ee0::EditOpState
{
public:
	NodeRotate3State(const pt0::Camera& cam, const pt3::Viewport& vp,
		const ee0::SubjectMgrPtr& sub_mgr, const ee0::SelectionSet<ee0::GameObjWithPos>& selection);

	virtual bool OnMousePress(int x, int y) override;
	virtual bool OnMouseRelease(int x, int y) override;
	virtual bool OnMouseDrag(int x, int y) override;

	virtual bool OnActive() override;
	virtual bool OnDraw() const override;

private:
	enum PointQueryType
	{
		POINT_QUERY_NULL = 0,
		POINT_QUERY_X,
		POINT_QUERY_Y,
		POINT_QUERY_Z,
	};
	PointQueryType PointQuery(int x, int y) const;

	void UpdateSelectionSetInfo();

	void Rotate(const sm::vec2& start, const sm::vec2& end);

	void DrawEdges() const;
	void DrawNodes() const;

private:
	const pt0::Camera&   m_cam;
	const pt3::Viewport& m_vp;

	ee0::SubjectMgrPtr m_sub_mgr;

	const ee0::SelectionSet<ee0::GameObjWithPos>& m_selection;

	pt2::OrthoCamera m_cam2d;

	sm::vec2 m_last_pos;

	PointQueryType m_op_type;

	sm::vec3 m_center;
	sm::vec2 m_center2d;

}; // NodeRotate3State

}