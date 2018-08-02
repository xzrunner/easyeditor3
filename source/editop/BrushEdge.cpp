#include "ee3/BrushEdge.h"

#include <quake/MapModel.h>
#include <model/MapLoader.h>
#include <painting3/Viewport.h>

namespace ee3
{
namespace mesh
{

void BrushEdge::Project(const pt3::Viewport& vp, const sm::mat4& cam_mat,
	                    std::function<void(const sm::vec2& b, const sm::vec2& e, const sm::vec2& mid)> func) const
{
	auto b3 = begin->pos * model::MapLoader::VERTEX_SCALE;
	auto e3 = end->pos * model::MapLoader::VERTEX_SCALE;
	auto b2 = vp.TransPosProj3ToProj2(b3, cam_mat);
	auto e2 = vp.TransPosProj3ToProj2(e3, cam_mat);
	auto mid2 = vp.TransPosProj3ToProj2((b3 + e3) * 0.5f, cam_mat);
	func(b2, e2, mid2);
}

}
}