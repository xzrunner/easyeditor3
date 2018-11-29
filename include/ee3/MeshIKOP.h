#pragma once

#include <ee0/EditOP.h>

namespace model { struct Model; class MeshIK; }
namespace ee0 { class WxStageCanvas; }
namespace pt3 { class Viewport; }

namespace ee3
{

class MeshIKOP : public ee0::EditOP
{
public:
	MeshIKOP(const std::shared_ptr<pt0::Camera>& camera,
		const std::shared_ptr<ee0::WxStageCanvas>& canvas,
		const pt3::Viewport& vp);

	virtual bool OnMouseLeftDown(int x, int y) override;
	virtual bool OnMouseRightDown(int x, int y) override;
	virtual bool OnMouseRightUp(int x, int y) override;
	virtual bool OnMouseDrag(int x, int y) override;

	virtual bool OnDraw() const;
	virtual bool Update(float dt);

	void SetModel(const std::shared_ptr<model::Model>& model);

private:
	void PrepareDeform() const;

	void QueryByPos(int x, int y);

	void DrawGUI() const;
	void DebugDraw() const;

private:
	std::shared_ptr<ee0::WxStageCanvas> m_canvas;
	const pt3::Viewport& m_vp;

	std::shared_ptr<model::Model> m_model = nullptr;

	mutable std::unique_ptr<model::MeshIK> m_ik = nullptr;

	int m_main_handle = 2096;
	mutable float m_handle_region_size = 15;
	mutable float m_unconstrained_region_size = 35;

}; // MeshIKOP

}