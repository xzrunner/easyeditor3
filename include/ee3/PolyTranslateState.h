#pragma once

#include "ee3/MeshPointQuery.h"

#include <ee0/EditOpState.h>
#include <ee0/typedef.h>

namespace pt3 { class Viewport; }

namespace ee3
{
namespace mesh
{

class PolyTranslateState : public ee0::EditOpState
{
public:
	PolyTranslateState(const std::shared_ptr<pt0::Camera>& camera, const pt3::Viewport& vp,
		const ee0::SubjectMgrPtr& sub_mgr, const MeshPointQuery::Selected& selected,
		std::function<void()> update_cb);

	virtual bool OnKeyPress(int key_code) override;
	virtual bool OnKeyRelease(int key_code) override;
	virtual bool OnMousePress(int x, int y) override;
	virtual bool OnMouseRelease(int x, int y) override;
	virtual bool OnMouseDrag(int x, int y) override;

	virtual bool OnDraw() const override;

	bool IsMoveAnyDirection() const;

private:
	void CalcTranslatePlane(const sm::Ray& ray, sm::Plane& plane) const;

	void TranslateSelected(const sm::vec3& offset);

private:
	// Axis Restriction
	enum MoveType
	{
		MOVE_ANY = 0,
		MOVE_X,
		MOVE_Y,
		MOVE_Z,
	};

private:
	const pt3::Viewport& m_vp;
	ee0::SubjectMgrPtr   m_sub_mgr;

	const MeshPointQuery::Selected& m_selected;

	sm::vec3 m_first_pos3;
	sm::vec3 m_last_pos3;

	sm::ivec2 m_first_pos2;
	sm::ivec2 m_last_pos2;

	bool m_move_fixed_xz;

	MoveType m_move_type;

	std::function<void()> m_update_cb;

}; // PolyTranslateState

}
}