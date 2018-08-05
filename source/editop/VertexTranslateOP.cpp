#include "ee3/VertexTranslateOP.h"

#include <model/Model.h>

namespace ee3
{
namespace mesh
{

VertexTranslateOP::VertexTranslateOP(const std::shared_ptr<pt0::Camera>& camera,
	                                 const pt3::Viewport& vp,
	                                 const ee0::SubjectMgrPtr& sub_mgr,
	                                 const MeshPointQuery::Selected& selected,
	                                 const ee0::SelectionSet<quake::BrushVertexPtr>& selection,
	                                 std::function<void()> update_cb)
	: MeshTranslateBaseOP<quake::BrushVertexPtr>(camera, vp, sub_mgr, selected, selection, update_cb)
{
}

bool VertexTranslateOP::QueryByPos(const sm::vec2& pos, const quake::BrushVertexPtr& vert,
	                               const sm::mat4& cam_mat) const
{
	auto p3 = vert->pos * model::MapLoader::VERTEX_SCALE;
	auto p2 = m_vp.TransPosProj3ToProj2(p3, cam_mat);
	if (sm::dis_pos_to_pos(p2, pos) < NODE_QUERY_RADIUS) {
		m_last_pos3 = p3;
		return true;
	} else {
		return false;
	}
}

void VertexTranslateOP::TranslateSelected(const sm::vec3& offset)
{
	auto& vertices = m_selected.poly->GetVertices();
	auto _offset = offset / model::MapLoader::VERTEX_SCALE;
	m_selection.Traverse([&](const quake::BrushVertexPtr& vert)->bool
	{
		// update helfedge geo
		for (auto& v : vertices)
		{
			auto d = vert->pos * model::MapLoader::VERTEX_SCALE - v->position;
			if (fabs(d.x) < SM_LARGE_EPSILON &&
				fabs(d.y) < SM_LARGE_EPSILON &&
				fabs(d.z) < SM_LARGE_EPSILON) {
				v->position += offset;
				break;
			}
		}

		// update quake map brush
		vert->pos += _offset;

		return true;
	});

	// update helfedge geo
	m_selected.poly->UpdateAABB();

	// update model aabb
	sm::cube model_aabb;
	model_aabb.Combine(m_selected.GetBrush()->geometry->GetAABB());
	m_selected.model->aabb = model_aabb;

	// update vbo
	model::MapLoader::UpdateVBO(*m_selected.model, m_selected.brush_idx);
}

}
}