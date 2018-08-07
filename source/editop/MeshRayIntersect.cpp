#include "ee3/MeshRayIntersect.h"

#include <SM_Ray.h>
#include <SM_Plane.h>
#include <SM_RayIntersect.h>
#include <painting3/PerspCam.h>
#include <painting3/Viewport.h>

namespace ee3
{
namespace mesh
{

bool ray_yline_intersect(const pt3::PerspCam& cam, const pt3::Viewport& vp, 
	                     const sm::vec3& line_pos, const sm::ivec2& ray_pos, sm::vec3& cross)
{
	sm::vec3 n = cam.GetPos() - line_pos;
	n.y = 0;
	n.Normalize();

	sm::vec3 ray_dir = vp.TransPos3ScreenToDir(
		sm::vec2(static_cast<float>(ray_pos.x), static_cast<float>(ray_pos.y)), cam);
	sm::Ray ray(cam.GetPos(), ray_dir);

	sm::Plane plane;
	// n.x * line_pos.x + n.z * line_pos.z + d = 0
	plane.Build(n, -line_pos.x * n.x - line_pos.z * n.z);

	if (!sm::ray_plane_intersect(ray, plane, &cross)) {
		return false;
	}

	cross.x = line_pos.x;
	cross.z = line_pos.z;

	return true;
}

}
}
