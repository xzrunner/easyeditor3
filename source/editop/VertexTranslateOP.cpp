#include "ee3/VertexTranslateOP.h"

namespace ee3
{
namespace mesh
{

VertexTranslateOP::VertexTranslateOP(const std::shared_ptr<pt0::Camera>& camera,
	                                 const pt3::Viewport& vp,
	                                 const ee0::SubjectMgrPtr& sub_mgr,
	                                 const MeshPointQuery::Selected& selected,
	                                 const ee0::SelectionSet<quake::BrushVertexPtr>& selection)
	: MeshTranslateBaseOP<quake::BrushVertexPtr>(camera, vp, sub_mgr, selected, selection)
{
}

bool VertexTranslateOP::QueryByPos(const sm::vec2& pos, const quake::BrushVertexPtr& vert,
	                               const sm::mat4& cam_mat) const
{
	auto p3 = vert->pos * model::MapLoader::VERTEX_SCALE;
	auto p2 = m_vp.TransPosProj3ToProj2(p3, cam_mat);
	if (sm::dis_pos_to_pos(p2, pos) < NODE_QUERY_RADIUS) {
		m_last_pos = p3;
		return true;
	} else {
		return false;
	}
}

void VertexTranslateOP::TranslateSelected(const sm::vec3& offset)
{
	m_selection.Traverse([&](const quake::BrushVertexPtr& vert)->bool
	{
		vert->pos += offset / model::MapLoader::VERTEX_SCALE;
		return true;
	});

	// update vbo
	model::MapLoader::UpdateVBO(*m_selected.model, m_selected.brush_idx);
}

}
}