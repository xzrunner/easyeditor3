#pragma once

#include <ee0/EditOP.h>
#include <ee0/typedef.h>

namespace pt3 { class ICamera; class Viewport; }

namespace ee3
{

class VertexTranslateOP : public ee0::EditOP
{
public:
	VertexTranslateOP(pt3::ICamera& cam, const pt3::Viewport& vp,
		const ee0::SubjectMgrPtr& sub_mgr);

	virtual bool OnKeyDown(int key_code) override;
	virtual bool OnKeyUp(int key_code) override;

	virtual bool OnMouseDrag(int x, int y) override;

private:
	const pt3::ICamera&  m_cam;
	const pt3::Viewport& m_vp;
	ee0::SubjectMgrPtr   m_sub_mgr;

}; // VertexTranslateOP

}