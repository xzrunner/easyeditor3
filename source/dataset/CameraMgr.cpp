#include "ee3/CameraMgr.h"

#include <painting3/OrthoCam.h>

namespace ee3
{

CameraMgr::CameraMgr(bool only3d)
	: m_curr(CAM_3D)
{
	assert(m_cams[CAM_3D] == nullptr);

	if (!only3d)
	{
		m_cams[CAM_XZ] = std::make_shared<pt3::OrthoCam>(pt3::OrthoCam::VP_XZ);
		m_cams[CAM_XY] = std::make_shared<pt3::OrthoCam>(pt3::OrthoCam::VP_XY);
		m_cams[CAM_ZY] = std::make_shared<pt3::OrthoCam>(pt3::OrthoCam::VP_ZY);
	}
}

const pt0::CameraPtr& CameraMgr::SwitchToNext()
{
	m_curr = static_cast<CameraType>((m_curr + 1) % CAM_MAX_COUNT);
	return m_cams[m_curr];
}

}