#pragma once

#include <ee0/EditOpState.h>
#include <ee0/SelectionSet.h>
#include <ee0/typedef.h>
#include <ee0/GameObj.h>

#include <SM_Vector.h>
#include <SM_Matrix.h>
#include <SM_Ray.h>
#include <painting2/OrthoCamera.h>

namespace pt3 { class ICamera; class Viewport; }

namespace ee3
{

class NodeTranslate3State : public ee0::EditOpState
{
public:
	NodeTranslate3State(const pt3::ICamera& cam, const pt3::Viewport& vp,
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

	void Translate(const sm::vec2& start, const sm::vec2& end);

	void DrawEdges() const;
	void DrawNodes() const;

	enum AxisNodeType
	{
		AXIS_CENTER = 0,
		AXIS_X,
		AXIS_Y,
		AXIS_Z,
	};

	sm::vec2 GetCtrlPos2D(const sm::mat4& cam_mat, AxisNodeType type) const;
	sm::vec3 GetCtrlPos3D(AxisNodeType type) const;

private:
	const pt3::ICamera&   m_cam;
	const pt3::Viewport& m_vp;

	ee0::SubjectMgrPtr m_sub_mgr;

	const ee0::SelectionSet<ee0::GameObjWithPos>& m_selection;

	pt2::OrthoCamera m_cam2d;

	// move path 2d
	sm::vec2 m_first_pos2;
	sm::vec2 m_first_dir2;

	sm::Ray m_move_path3d;

	sm::vec2 m_last_pos2;
	sm::vec3 m_last_pos3;

	PointQueryType m_op_type;

	// selection set info

	sm::vec3 m_center;
	sm::vec2 m_center2d;

	sm::mat4 m_rot_mat;

}; // NodeTranslate3State

}