#pragma once

#include "ee3/PolySelectOP.h"

#include <SM_Plane.h>

namespace ee3
{

class PolyArrangeOP : public ee3::PolySelectOP
{
public:
	PolyArrangeOP(ee0::WxStagePage& stage, pt3::Camera& cam,
		const pt3::Viewport& vp);

	virtual bool OnKeyDown(int key_code) override;
	virtual bool OnKeyUp(int key_code) override;

	virtual bool OnMouseLeftDown(int x, int y) override;
	virtual bool OnMouseLeftUp(int x, int y) override;
	virtual bool OnMouseDrag(int x, int y) override;

	virtual bool OnDraw() const override;

private:
	void CalcTranslatePlane(const sm::Ray& ray, sm::Plane& plane) const;

	void TranslateSelected(const sm::vec3& offset);

private:
	const pt3::Camera&   m_cam;
	const pt3::Viewport& m_vp;

	sm::vec3 m_last_pos;

	ee0::EditOpStatePtr m_face_pp_state = nullptr;

}; // PolyArrangeOP

}