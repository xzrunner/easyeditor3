#pragma once

namespace ee0 { class SubjectMgr; }

namespace ee3
{

class MsgHelper
{
public:
	static void SelectSkeletalJoint(ee0::SubjectMgr& sub_mgr, int joint_id);

}; // MsgHelper

}