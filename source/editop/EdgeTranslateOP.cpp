#include "ee3/EdgeTranslateOP.h"

namespace ee3
{
namespace mesh
{

EdgeTranslateOP::EdgeTranslateOP(const std::shared_ptr<pt0::Camera>& camera,
	                             const pt3::Viewport& vp,
	                             const ee0::SubjectMgrPtr& sub_mgr,
	                             const MeshPointQuery::Selected& selected,
	                             const ee0::SelectionSet<BrushEdge>& selection)
	: MeshTranslateBaseOP<BrushEdge>(camera, vp, sub_mgr, selected, selection)
{
}

bool EdgeTranslateOP::QueryByPos(const sm::vec2& pos, const BrushEdge& edge,
	                             const sm::mat4& cam_mat) const
{
	auto b3 = edge.begin->pos * model::MapLoader::VERTEX_SCALE;
	auto e3 = edge.end->pos * model::MapLoader::VERTEX_SCALE;
	auto mid3 = (b3 + e3) * 0.5f;
	auto b2 = m_vp.TransPosProj3ToProj2(b3, cam_mat);
	auto e2 = m_vp.TransPosProj3ToProj2(e3, cam_mat);
	auto mid2 = m_vp.TransPosProj3ToProj2(mid3, cam_mat);
	if (sm::dis_pos_to_pos(mid2, pos) < NODE_QUERY_RADIUS) {
		m_last_pos = mid3;
		return true;
	} else {
		return false;
	}
}

void EdgeTranslateOP::TranslateSelected(const sm::vec3& offset)
{
	auto _offset = offset / model::MapLoader::VERTEX_SCALE;
	m_selection.Traverse([&](const BrushEdge& edge)->bool
	{
		edge.begin->pos += _offset;
		edge.end->pos += _offset;
		return true;
	});

	// update vbo
	model::MapLoader::UpdateVBO(*m_selected.model, m_selected.brush_idx);
}

}
}