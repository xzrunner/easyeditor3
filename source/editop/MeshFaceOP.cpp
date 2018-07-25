#include "ee3/MeshFaceOP.h"

#include <ee0/WxStagePage.h>
#include <ee0/SubjectMgr.h>

#include <model/Model.h>
#include <model/HalfEdgeMesh.h>
#include <painting3/PrimitiveDraw.h>
#include <painting3/Camera.h>
#include <painting3/Viewport.h>
#include <halfedge/HalfEdge.h>
#include <node0/SceneNode.h>
#include <node3/CompModel.h>
#include <node3/CompModelInst.h>
#include <node3/CompTransform.h>
#include <node3/Math.h>
#include <painting3/Ray.h>

namespace ee3
{

MeshFaceOP::MeshFaceOP(ee0::WxStagePage& stage, pt3::Camera& cam,
	                   const pt3::Viewport& vp)
	: WorldTravelOP(cam, vp, stage.GetSubjectMgr())
	, m_vp(vp)
	, m_selection(stage.GetSelection())
{
}

bool MeshFaceOP::OnMouseLeftDown(int x, int y)
{
	if (WorldTravelOP::OnMouseLeftDown(x, y)) {
		return true;
	}

	PointQuery(x, y);

	m_sub_mgr->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);

	return false;
}

bool MeshFaceOP::OnDraw() const
{
	if (WorldTravelOP::OnDraw()) {
		return true;
	}

	if (!m_selected_poly) {
		return false;
	}

	//pt3::PrimitiveDraw::SetColor(0xff0000ff);
	//pt3::PrimitiveDraw::Cube(m_selected_mat, m_selected_poly->GetAABB());

	if (!m_selected_face) {
		return false;
	}

	pt3::PrimitiveDraw::SetColor(0xff0000ff);
	std::vector<sm::vec3> polyline;
	m_selected_face->GetBorder(polyline);
	for (int i = 0, n = polyline.size(); i < n; ++i) {
		polyline[i] = m_selected_mat * polyline[i];
	}
	pt3::PrimitiveDraw::Polyline(polyline, true);
//	pt3::PrimitiveDraw::Polygon(polyline);

	return false;
}

void MeshFaceOP::PointQuery(int x, int y)
{
	m_selected_poly = nullptr;
	m_selected_face = nullptr;

	auto cam_mat = m_cam.GetModelViewMat() * m_cam.GetProjectionMat();

	sm::vec3 ray_dir = m_vp.TransPos3ScreenToDir(sm::vec2(x, y), m_cam);
	pt3::Ray ray(m_cam.GetPos(), ray_dir);
	m_selection.Traverse([&](const ee0::GameObjWithPos& opw)->bool
	{
		auto& node = opw.GetNode();

		auto& cmodel = node->GetUniqueComp<n3::CompModelInst>();
		auto& model = cmodel.GetModel();
		if (!model || !model->GetModel()) {
			return false;
		}

		sm::vec3 cross;
		auto& ctrans = node->GetUniqueComp<n3::CompTransform>();
		if (!n3::Math::RayOBBIntersection(model->GetModel()->aabb, ctrans.GetPosition(),
			ctrans.GetAngle(), ctrans.GetScale(), ray, &cross)) {
			return false;
		}

		auto& ext = model->GetModel()->ext;
		if (!ext || ext->Type() != model::EXT_HALFEDGE_MESH) {
			return false;
		}
		auto he_mesh = static_cast<model::HalfEdgeMesh*>(ext.get());
		for (auto& mesh : he_mesh->meshes)
		{
			if (n3::Math::RayOBBIntersection(mesh->GetAABB(), ctrans.GetPosition(),
				ctrans.GetAngle(), ctrans.GetScale(), ray, &cross))
			{
				m_selected_poly = mesh;
				m_selected_mat = ctrans.GetTransformMat();

				auto& faces = mesh->GetFaces();
				for (auto& face : faces) {
					std::vector<sm::vec3> border;
					face->GetBorder(border);
					sm::vec3 cross_face;
					if (n3::Math::RayPolygonIntersection(
						m_selected_mat, border, ray, &cross_face))
					{
						m_selected_face = face;
						break;
					}
				}

				return true;
			}
		}

		return false;
	});
}

}