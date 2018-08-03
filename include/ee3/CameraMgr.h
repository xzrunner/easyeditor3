#pragma once

#include <painting0/Camera.h>

namespace ee3
{

class CameraMgr
{
public:
	enum CameraType
	{
		CAM_3D = 0,
		CAM_XZ,
		CAM_XY,
		CAM_ZY,

		CAM_MAX_COUNT,
	};

public:
	CameraMgr(bool only3d = true);

	const pt0::CameraPtr& GetCamera(CameraType type) const {
		return m_cams[type];
	}
	void SetCamera(const pt0::CameraPtr& cam, CameraType type) {
		m_cams[type] = cam;
	}

	const pt0::CameraPtr& GetCamera() const { return m_cams[m_curr]; }

	const pt0::CameraPtr& SwitchToNext();

private:
	pt0::CameraPtr m_cams[CAM_MAX_COUNT];

	CameraType m_curr;

}; // CameraMgr

}