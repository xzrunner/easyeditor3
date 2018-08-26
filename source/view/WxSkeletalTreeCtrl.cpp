#include "ee3/WxSkeletalTreeCtrl.h"
#include "ee3/MessageID.h"

#include <ee0/SubjectMgr.h>

#include <model/SkeletalAnim.h>

namespace ee3
{

BEGIN_EVENT_TABLE(WxSkeletalTreeCtrl, wxTreeCtrl)
END_EVENT_TABLE()

WxSkeletalTreeCtrl::WxSkeletalTreeCtrl(wxWindow* parent, const ee0::SubjectMgrPtr& sub_mgr)
	: wxTreeCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
	wxTR_EDIT_LABELS | wxTR_MULTIPLE | wxTR_NO_LINES | wxTR_DEFAULT_STYLE)
	, m_sub_mgr(sub_mgr)
{
	Bind(wxEVT_TREE_SEL_CHANGED, &WxSkeletalTreeCtrl::OnSelChanged, this, GetId());
}

void WxSkeletalTreeCtrl::LoadFromSkeletal(const ::model::SkeletalAnim& skeletal)
{
	auto& nodes = skeletal.GetAllNodes();
	if (nodes.empty()) {
		return;
	}

	m_items.reserve(nodes.size());
	for (int i = 0, n = nodes.size(); i < n; ++i) {
		m_items.emplace_back(i);
	}

	m_root = AddRoot(nodes[0]->name);
	SetItemData(m_root, &m_items[0]);
	for (auto& c : nodes[0]->children) {
		InsertNode(m_root, skeletal, c);
	}

	ExpandAll();
}

void WxSkeletalTreeCtrl::InsertNode(wxTreeItemId parent, const ::model::SkeletalAnim& skeletal, int child)
{
	auto& nodes = skeletal.GetAllNodes();
	auto& node = nodes[child];
	auto id = InsertItem(parent, 0, node->name);
	SetItemData(id, &m_items[child]);
	for (auto& c : node->children) {
		InsertNode(id, skeletal, c);
	}
}

void WxSkeletalTreeCtrl::OnSelChanged(wxTreeEvent& event)
{
	auto id = event.GetItem();
	if (!id.IsOk()) {
		return;
	}

	auto pdata = (Item*)GetItemData(id);

	ee0::VariantSet vars;

	ee0::Variant var_joint;
	var_joint.m_type = ee0::VT_INT;
	var_joint.m_val.l = pdata->m_node_id;
	vars.SetVariant("joint_id", var_joint);

	m_sub_mgr->NotifyObservers(MSG_SKELETAL_TREE_ON_SELECT, vars);
}

}