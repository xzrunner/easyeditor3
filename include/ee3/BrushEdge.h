#pragma once

#include <quake/MapModel.h>
#include <SM_Matrix.h>

#include <functional>

namespace pt3 { class Viewport; }

namespace ee3
{
namespace mesh
{

struct BrushEdge
{
	BrushEdge() : begin(nullptr), end(nullptr) {}
	BrushEdge(const quake::BrushVertexPtr& begin, const quake::BrushVertexPtr& end)
		: begin(begin), end(end) {}

	bool operator == (const BrushEdge& e) const {
		return begin == e.begin && end == e.end;
	}
	operator bool() const {
		return begin && end;
	}

	void Project(const pt3::Viewport& vp, const sm::mat4& cam_mat,
		std::function<void(const sm::vec2& b, const sm::vec2& e, const sm::vec2& mid)> func) const;

	quake::BrushVertexPtr begin;
	quake::BrushVertexPtr end;

}; // BrushEdge

}
}