#pragma once

#include <ee0/EditOP.h>
#include <ee0/GameObj.h>
#include <ee0/SelectionSet.h>
#include <ee0/typedef.h>

#include <painting2/OrthoCamera.h>

namespace ee0 { class WxStagePage; }
namespace pt3 { class Camera; class Viewport; }

namespace ee3
{

class NodeRotateOP : public ee0::EditOP
{
public:
	NodeRotateOP(ee0::WxStagePage& stage, pt3::Camera& cam,
		const pt3::Viewport& vp);

	virtual bool OnMouseLeftDown(int x, int y) override;
	virtual bool OnMouseLeftUp(int x, int y) override;
	virtual bool OnMouseDrag(int x, int y);
	virtual bool OnMouseWheelRotation(int x, int y, int direction) override;

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

	void InitSelectionCenter();

	void Rotate(const sm::vec2& start, const sm::vec2& end);

private:
	ee0::SelectionSet<ee0::GameObjWithPos>& m_selection;
	ee0::SubjectMgrPtr m_sub_mgr;

	pt3::Camera& m_cam;
	const pt3::Viewport& m_vp;

	pt2::OrthoCamera m_cam2d;

	std::shared_ptr<ee0::EditOpState> m_cam_zoom_state;

	PointQueryType m_op_type;

	sm::vec2 m_last_pos;

	sm::vec3 m_center;
	sm::vec2 m_center2d;

}; // NodeRotateOP

}