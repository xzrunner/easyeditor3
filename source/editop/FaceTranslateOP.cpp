#include "ee3/FaceTranslateOP.h"

namespace ee3
{
namespace mesh
{

FaceTranslateOP::FaceTranslateOP(const std::shared_ptr<pt0::Camera>& camera,
	                             const pt3::Viewport& vp,
	                             const ee0::SubjectMgrPtr& sub_mgr,
	                             const MeshPointQuery::Selected& selected,
	                             const ee0::SelectionSet<quake::BrushFacePtr>& selection)
	: MeshTranslateBaseOP<quake::BrushFacePtr>(camera, vp, sub_mgr, selected, selection)
{
}

bool FaceTranslateOP::QueryByPos(const sm::vec2& pos, const quake::BrushFacePtr& face,
	                             const sm::mat4& cam_mat) const
{
	assert(!face->vertices.empty());
	sm::vec3 c3;
	for (auto& v : face->vertices) {
		c3 += v->pos;
	}
	c3 /= static_cast<float>(face->vertices.size());
	c3 *= model::MapLoader::VERTEX_SCALE;
	auto c2 = m_vp.TransPosProj3ToProj2(c3, cam_mat);
	if (sm::dis_pos_to_pos(c2, pos) < NODE_QUERY_RADIUS) {
		m_last_pos = c3;
		return true;
	} else {
		return false;
	}
}

void FaceTranslateOP::TranslateSelected(const sm::vec3& offset)
{
	auto _offset = offset / model::MapLoader::VERTEX_SCALE;
	m_selection.Traverse([&](const quake::BrushFacePtr& face)->bool {
		for (auto& v : face->vertices) {
			v->pos += _offset;
		}
		return true;
	});

	// update vbo
	model::MapLoader::UpdateVBO(*m_selected.model, m_selected.brush_idx);
}

}
}