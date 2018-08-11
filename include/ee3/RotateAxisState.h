#pragma once

#include <ee0/EditOpState.h>
#include <ee0/typedef.h>
#include <ee0/GameObj.h>

#include <SM_Vector.h>
#include <SM_Matrix.h>
#include <SM_Quaternion.h>

#include <functional>

namespace pt2 { class OrthoCamera; }
namespace pt3 { class Viewport; }

namespace ee3
{

class RotateAxisState : public ee0::EditOpState
{
public:
	struct Callback
	{
		std::function<bool()>                      is_need_draw;
		std::function<void(sm::vec3&, sm::mat4&)>  get_origin_transform;
		std::function<void(const sm::Quaternion&)> rotate;
	};

	struct Config
	{
		Config(float arc_radius = 5, float node_radius = 5)
			: arc_radius(arc_radius), node_radius(node_radius) {}

		float arc_radius;
		float node_radius;
	};

public:
	RotateAxisState(const std::shared_ptr<pt0::Camera>& camera, const pt3::Viewport& vp,
		const ee0::SubjectMgrPtr& sub_mgr, const Callback& cb, const Config& cfg);

	virtual bool OnMousePress(int x, int y) override;
	virtual bool OnMouseRelease(int x, int y) override;
	virtual bool OnMouseDrag(int x, int y) override;

	virtual bool OnActive(bool active) override;
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
	const pt3::Viewport& m_vp;

	ee0::SubjectMgrPtr m_sub_mgr;

	Callback m_cb;
	Config   m_cfg;

	std::shared_ptr<pt2::OrthoCamera> m_cam2d;

	sm::vec2 m_last_pos;

	PointQueryType m_op_type;

	sm::vec3 m_pos;
	sm::vec2 m_pos2d;
	sm::mat4 m_rotate;

	bool m_active = true;

}; // RotateAxisState

}