#include "ee3/EdgeTranslateOP.h"

#include <model/Model.h>

namespace ee3
{
namespace mesh
{

EdgeTranslateOP::EdgeTranslateOP(const std::shared_ptr<pt0::Camera>& camera,
	                             const pt3::Viewport& vp,
	                             const ee0::SubjectMgrPtr& sub_mgr,
	                             const MeshPointQuery::Selected& selected,
	                             const ee0::SelectionSet<BrushEdge>& selection,
	                             std::function<void()> update_cb)
	: MeshTranslateBaseOP<BrushEdge>(camera, vp, sub_mgr, selected, selection, update_cb)
{
}

bool EdgeTranslateOP::QueryByPos(const sm::vec2& pos, const BrushEdge& edge,
	                             const sm::mat4& cam_mat) const
{
	auto b3 = edge.begin->pos * model::MapBuilder::VERTEX_SCALE;
	auto e3 = edge.end->pos * model::MapBuilder::VERTEX_SCALE;
	auto mid3 = (b3 + e3) * 0.5f;
	auto b2 = m_vp.TransPosProj3ToProj2(b3, cam_mat);
	auto e2 = m_vp.TransPosProj3ToProj2(e3, cam_mat);
	auto mid2 = m_vp.TransPosProj3ToProj2(mid3, cam_mat);
	if (sm::dis_pos_to_pos(mid2, pos) < NODE_QUERY_RADIUS) {
		m_last_pos3 = mid3;
		return true;
	} else {
		return false;
	}
}

void EdgeTranslateOP::TranslateSelected(const sm::vec3& offset)
{
	auto& faces = m_selected.poly->GetFaces();
	auto _offset = offset / model::MapBuilder::VERTEX_SCALE;
	m_selection.Traverse([&](const BrushEdge& edge)->bool
	{
		// update helfedge geo
		for (auto& f : faces)
		{
			auto start = f->start_edge;
			auto curr = start;
			do {
				auto d0 = edge.begin->pos * model::MapBuilder::VERTEX_SCALE - curr->origin->position;
				auto d1 = edge.end->pos * model::MapBuilder::VERTEX_SCALE - curr->next->origin->position;
				if (fabs(d0.x) < SM_LARGE_EPSILON &&
					fabs(d0.y) < SM_LARGE_EPSILON &&
					fabs(d0.z) < SM_LARGE_EPSILON &&
					fabs(d1.x) < SM_LARGE_EPSILON &&
					fabs(d1.y) < SM_LARGE_EPSILON &&
					fabs(d1.z) < SM_LARGE_EPSILON) {
					curr->origin->position += offset;
					curr->next->origin->position += offset;
					break;
				}
				curr = curr->next;
			} while (curr != start);
		}

		// update quake map brush
		edge.begin->pos += _offset;
		edge.end->pos += _offset;

		return true;
	});

	// update helfedge geo
	m_selected.poly->UpdateAABB();

	// update model aabb
	sm::cube model_aabb;
	model_aabb.Combine(m_selected.GetBrush()->geometry->GetAABB());
	m_selected.model->aabb = model_aabb;

	// update vbo
	model::MapBuilder::UpdateVBO(*m_selected.model, m_selected.brush_idx);
}

}
}