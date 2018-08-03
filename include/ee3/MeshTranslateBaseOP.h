#pragma once

#include "ee3/MeshPointQuery.h"

#include <ee0/EditOP.h>
#include <ee0/typedef.h>
#include <ee0/SelectionSet.h>

#include <quake/MapModel.h>

namespace pt2 { class OrthoCamera; }
namespace pt3 { class Viewport; }

namespace ee3
{
namespace mesh
{

template <typename T>
class MeshTranslateBaseOP : public ee0::EditOP
{
public:
	MeshTranslateBaseOP(const std::shared_ptr<pt0::Camera>& camera, const pt3::Viewport& vp,
		const ee0::SubjectMgrPtr& sub_mgr, const MeshPointQuery::Selected& selected,
		const ee0::SelectionSet<T>& selection);

	virtual bool OnKeyDown(int key_code) override;
	virtual bool OnMouseLeftDown(int x, int y) override;
	virtual bool OnMouseLeftUp(int x, int y) override;
	virtual bool OnMouseDrag(int x, int y) override;

protected:
	virtual bool QueryByPos(const sm::vec2& pos, const T& data,
		const sm::mat4& cam_mat) const = 0;
	virtual void TranslateSelected(const sm::vec3& offset) = 0;

protected:
	const pt3::Viewport& m_vp;

	ee0::SubjectMgrPtr   m_sub_mgr;

	const MeshPointQuery::Selected& m_selected;
	const ee0::SelectionSet<T>&     m_selection;

	std::shared_ptr<pt2::OrthoCamera> m_cam2d;

	mutable sm::vec3 m_last_pos;

}; // MeshTranslateBaseOP

}
}

#include "ee3/MeshTranslateBaseOP.inl"