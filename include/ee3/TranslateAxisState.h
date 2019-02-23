#pragma once

#include <ee0/EditOpState.h>
#include <ee0/typedef.h>
#include <ee0/GameObj.h>

#include <SM_Vector.h>
#include <SM_Matrix.h>
#include <SM_Quaternion.h>
#include <SM_Ray.h>

#include <functional>

namespace pt2 { class OrthoCamera; }
namespace pt3 { class Viewport; }

namespace ee3
{

class TranslateAxisState : public ee0::EditOpState
{
public:
	struct Callback
	{
		std::function<bool()>                is_need_draw;
		std::function<sm::mat4()>            get_origin_wmat;
		std::function<void(const sm::vec3&)> translate;
	};

	struct Config
	{
		Config(float arc_radius = 5, float node_radius = 5)
			: arc_radius(arc_radius), node_radius(node_radius) {}

		float arc_radius;
		float node_radius;
	};

public:
	TranslateAxisState(const std::shared_ptr<pt0::Camera>& camera, const pt3::Viewport& vp,
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
        POINT_QUERY_YZ,
        POINT_QUERY_ZX,
        POINT_QUERY_XY,
	};
	PointQueryType PointQuery(int x, int y, sm::vec3& pos) const;

	void UpdateSelectionSetInfo();

	void Translate(int x, int y);

	enum AxisNodeType
	{
		AXIS_CENTER = 0,
		AXIS_X,
		AXIS_Y,
		AXIS_Z,
	};

	sm::vec2 GetCtrlPos2D(const sm::mat4& cam_mat, AxisNodeType type) const;
	sm::vec3 GetCtrlPos3D(AxisNodeType type) const;

    float CalcCoordAxisLen() const;

private:
	const pt3::Viewport& m_vp;

	ee0::SubjectMgrPtr m_sub_mgr;

	Callback m_cb;
	Config   m_cfg;

	std::shared_ptr<pt2::OrthoCamera> m_cam2d;

	// move path 2d
	sm::vec2 m_first_pos2;
	sm::vec2 m_first_dir2;

	sm::Ray m_move_path3d;

	sm::vec2 m_last_pos2;
	sm::vec3 m_last_pos3;

	PointQueryType m_op_type;

	sm::mat4 m_ori_wmat_scale;
    sm::mat4 m_ori_wmat_rotate;
    sm::mat4 m_ori_wmat_translate;

	sm::mat4 m_ori_wmat_no_scale;

}; // TranslateAxisState

}