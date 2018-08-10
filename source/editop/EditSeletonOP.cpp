#include "ee3/EditSeletonOP.h"

#include <model/Model.h>
#include <model/ModelInstance.h>
#include <painting2/PrimitiveDraw.h>
#include <painting3/PrimitiveDraw.h>

namespace ee3
{

EditSeletonOP::EditSeletonOP(const std::shared_ptr<pt0::Camera>& camera,
	                         ee0::WxStagePage& stage, const pt3::Viewport& vp)
	: ee0::EditOP(camera)
{
}

bool EditSeletonOP::OnMouseLeftDown(int x, int y)
{
	if (ee0::EditOP::OnMouseLeftDown(x, y)) {
		return true;
	}

	return false;
}

bool EditSeletonOP::OnMouseLeftUp(int x, int y)
{
	if (ee0::EditOP::OnMouseLeftUp(x, y)) {
		return true;
	}

	return false;
}

bool EditSeletonOP::OnMouseMove(int x, int y)
{
	if (ee0::EditOP::OnMouseMove(x, y)) {
		return true;
	}

	return false;
}

bool EditSeletonOP::OnMouseDrag(int x, int y)
{
	if (ee0::EditOP::OnMouseDrag(x, y)) {
		return true;
	}

	return false;
}

bool EditSeletonOP::OnDraw() const
{
	if (ee0::EditOP::OnDraw()) {
		return true;
	}

	if (!m_model) {
		return false;
	}

	auto& bones = (static_cast<::model::SkeletalAnim*>(m_model->GetModel()->ext.get())->GetAllNodes());

	auto& g_trans = m_model->GetGlobalTrans();
	for (int i = 0, n = bones.size(); i < n; ++i)
	{
		auto p_pos = g_trans[i] * sm::vec3(0, 0, 0);
		pt2::PrimitiveDraw::PointSize(5);
		pt3::PrimitiveDraw::SetColor(0xff0000ff);
		pt3::PrimitiveDraw::Point(p_pos);
		for (auto& child : bones[i]->children)
		{
			auto c_pos = g_trans[child] * sm::vec3(0, 0, 0);
			pt3::PrimitiveDraw::SetColor(0xff00ff00);
			pt3::PrimitiveDraw::Line(p_pos, c_pos);
		}
	}

	return false;
}

}