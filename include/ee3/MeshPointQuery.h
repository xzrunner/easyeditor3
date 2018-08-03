#pragma once

#include <ee0/SelectionSet.h>
#include <ee0/GameObj.h>

#include <SM_Matrix.h>
#include <SM_Ray.h>
#include <halfedge/HalfEdge.h>
#include <halfedge/Polyhedron.h>

namespace n3 { class CompTransform; }
namespace model { struct Model; }
namespace quake { struct MapBrush; }

namespace ee3
{
namespace mesh
{

class MeshPointQuery
{
public:
	struct Selected
	{
		Selected() {
			pos.MakeInvalid();
		}

		void Reset()
		{
			poly = nullptr;
			face = nullptr;
			pos.MakeInvalid();
			min_dist  = std::numeric_limits<float>::max();
			model     = nullptr;
			brush_idx = -1;
			node      = nullptr;
		}

		const quake::MapBrush* GetBrush() const;

		sm::mat4          mat;
		sm::vec3          normal;

		he::PolyhedronPtr poly = nullptr;
		he::FacePtr       face = nullptr;
		sm::vec3          pos;

		float             min_dist = std::numeric_limits<float>::max();

		std::shared_ptr<model::Model> model = nullptr;
		int               brush_idx         = -1;
		int               face_idx          = -1;

		n0::SceneNodePtr  node = nullptr;
	};

public:
	static bool Query(const ee0::GameObj& obj, const sm::Ray& ray, const sm::vec3& cam_pos, Selected& ret);

	static bool Query(const he::PolyhedronPtr& poly, const n3::CompTransform& poly_trans,
		const sm::Ray& ray, const sm::vec3& cam_pos, Selected& ret);

}; // MeshPointQuery

}
}