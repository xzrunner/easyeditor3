#include "ee3/MeshPointQuery.h"

#include <SM_Calc.h>
#include <SM_Ray.h>
#include <SM_RayIntersect.h>
#include <model/Model.h>
#include <model/QuakeMapEntity.h>
#include <node0/SceneNode.h>
#include <node3/CompModel.h>
#include <node3/CompModelInst.h>
#include <node3/CompTransform.h>

namespace ee3
{

bool MeshPointQuery::Query(const ee0::GameObj& obj, const sm::Ray& ray,
	                       const sm::vec3& cam_pos, Selected& ret)
{
	auto& node = obj;

	auto& cmodel = node->GetUniqueComp<n3::CompModelInst>();
	auto& model = cmodel.GetModel();
	if (!model || !model->GetModel()) {
		return false;
	}

	sm::vec3 cross;
	auto& ctrans = node->GetUniqueComp<n3::CompTransform>();
	if (!sm::ray_obb_intersect(model->GetModel()->aabb, ctrans.GetPosition(),
		ctrans.GetAngle(), ctrans.GetScale(), ray, &cross)) {
		return false;
	}

	auto& ext = model->GetModel()->ext;
	if (!ext || ext->Type() != model::EXT_QUAKE_MAP) {
		return false;
	}

	bool find = false;
	auto map_entity = static_cast<model::QuakeMapEntity*>(ext.get());
	auto& brushes = map_entity->GetMapEntity()->brushes;
	assert(brushes.size() == model->GetModel()->meshes.size());
	for (int i = 0, n = brushes.size(); i < n; ++i)
	{
		auto& brush = brushes[i];
		if (!Query(brush.geometry, ctrans, ray, cam_pos, ret)) {
			continue;
		}

		ret.model = model->GetModel();
		ret.brush_idx = i;

		ret.node = node;

		find = true;
	}
	return find;
}

bool MeshPointQuery::Query(const he::PolyhedronPtr& poly, const n3::CompTransform& poly_trans,
                           const sm::Ray& ray, const sm::vec3& cam_pos, Selected& ret)
{
	sm::vec3 cross;
	if (!sm::ray_obb_intersect(poly->GetAABB(), poly_trans.GetPosition(),
		poly_trans.GetAngle(), poly_trans.GetScale(), ray, &cross)) {
		return false;
	}

	float dist = sm::dis_pos3_to_pos3(cross, cam_pos);
	if (dist > ret.min_dist) {
		return false;
	}

	ret.poly = poly;
	ret.mat = poly_trans.GetTransformMat();

	bool find = false;
	auto& faces = poly->GetFaces();
	for (int i = 0, n = faces.size(); i < n; ++i)
	{
		auto& face = faces[i];

		std::vector<sm::vec3> border;
		face->GetBorder(border);
		assert(border.size() > 2);
		sm::vec3 cross_face;

		if (!sm::ray_polygon_intersect(
			ret.mat, border, ray, &cross_face)) {
			continue;
		}
		float dist = sm::dis_pos3_to_pos3(cross_face, cam_pos);
		if (dist >= ret.min_dist) {
			continue;
		}

		ret.min_dist = dist;

		ret.face = face;
		ret.face_idx = i;

		sm::vec3 intersect;
		sm::Plane face_plane(
			ret.mat * border[0],
			ret.mat * border[1],
			ret.mat * border[2]
		);
		if (sm::ray_plane_intersect(ray, face_plane, &intersect))
		{
			find = true;
			ret.pos = intersect;
			ret.normal = face_plane.normal;
		}
	}
	return find;
}

}