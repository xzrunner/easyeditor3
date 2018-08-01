#include "ee3/VertexTranslateOP.h"

namespace ee3
{

VertexTranslateOP::VertexTranslateOP(pt3::ICamera& cam,
	                       const pt3::Viewport& vp,
	                       const ee0::SubjectMgrPtr& sub_mgr)
	: m_cam(cam)
	, m_vp(vp)
	, m_sub_mgr(sub_mgr)
{
}

bool VertexTranslateOP::OnKeyDown(int key_code)
{
	if (ee0::EditOP::OnKeyDown(key_code)) {
		return true;
	}

	return false;
}

bool VertexTranslateOP::OnKeyUp(int key_code)
{
	if (ee0::EditOP::OnKeyUp(key_code)) {
		return true;
	}

	return false;
}

bool VertexTranslateOP::OnMouseDrag(int x, int y)
{
	if (ee0::EditOP::OnMouseDrag(x, y)) {
		return true;
	}

	return false;
}

}