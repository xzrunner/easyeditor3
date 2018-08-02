#pragma once

#include <painting3/ICamera.h>

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

	const pt3::ICameraPtr& GetCamera(CameraType type) const {
		return m_cams[type];
	}
	void SetCamera(const pt3::ICameraPtr& cam, CameraType type) {
		m_cams[type] = cam;
	}

	const pt3::ICameraPtr& GetCamera() const { return m_cams[m_curr]; }

	const pt3::ICameraPtr& SwitchToNext();

private:
	pt3::ICameraPtr m_cams[CAM_MAX_COUNT];

	CameraType m_curr;

}; // CameraMgr

}