#pragma once

#include <SM_Vector.h>

namespace pt3 { class Viewport; class PerspCam; }

namespace ee3
{
namespace mesh
{

bool ray_yline_intersect(const pt3::PerspCam& cam, const pt3::Viewport& vp, 
	                     const sm::vec3& line_pos, const sm::ivec2& ray_pos, sm::vec3& cross);

}
}
