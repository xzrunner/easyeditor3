#include "ee3/MsgHelper.h"
#include "ee3/MessageID.h"

#include <ee0/VariantSet.h>
#include <ee0/SubjectMgr.h>

namespace ee3
{

void MsgHelper::SelectSkeletalJoint(ee0::SubjectMgr& sub_mgr, int joint_id)
{
	ee0::VariantSet vars;

	ee0::Variant var_joint;
	var_joint.m_type = ee0::VT_INT;
	var_joint.m_val.l = joint_id;
	vars.SetVariant("joint_id", var_joint);

	sub_mgr.NotifyObservers(MSG_SKELETAL_TREE_ON_SELECT, vars);
}

}