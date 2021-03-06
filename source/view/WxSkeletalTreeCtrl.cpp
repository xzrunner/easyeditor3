#include "ee3/WxSkeletalTreeCtrl.h"
#include "ee3/MessageID.h"
#include "ee3/MsgHelper.h"

#include <ee0/SubjectMgr.h>

#include <model/SkeletalAnim.h>

namespace ee3
{

BEGIN_EVENT_TABLE(WxSkeletalTreeCtrl, wxTreeCtrl)
END_EVENT_TABLE()

WxSkeletalTreeCtrl::WxSkeletalTreeCtrl(wxWindow* parent, const ee0::SubjectMgrPtr& sub_mgr)
	: wxTreeCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
	wxTR_EDIT_LABELS | wxTR_SINGLE | wxTR_NO_LINES | wxTR_DEFAULT_STYLE)
	, m_sub_mgr(sub_mgr)
{
	Bind(wxEVT_TREE_SEL_CHANGED, &WxSkeletalTreeCtrl::OnSelChanged, this, GetId());

    m_messages.push_back(ee3::MSG_SKELETAL_TREE_ON_SELECT);

    for (auto& m : m_messages) {
        m_sub_mgr->RegisterObserver(m, this);
    }
}

WxSkeletalTreeCtrl::~WxSkeletalTreeCtrl()
{
    for (auto& m : m_messages) {
        m_sub_mgr->UnregisterObserver(m, this);
    }
}

void WxSkeletalTreeCtrl::OnNotify(uint32_t msg, const ee0::VariantSet& variants)
{
	switch (msg)
	{
	case ee3::MSG_SKELETAL_TREE_ON_SELECT:
		{
			int joint_id = variants.GetVariant("joint_id").m_val.l;
			for (auto& i : m_items)
			{
				if (i->m_node_id == joint_id) {
					SelectItem(i->GetId());
					break;
				}
			}
		}
	break;
	}
}

void WxSkeletalTreeCtrl::LoadFromSkeletal(const ::model::SkeletalAnim& skeletal)
{
    ClearAll();

	auto& nodes = skeletal.GetNodes();
	if (nodes.empty()) {
		return;
	}

	m_root = AddRoot(nodes[0]->name);
    auto item = new Item(0);
    m_items.push_back(item);
	SetItemData(m_root, item);
	for (auto& c : nodes[0]->children) {
		InsertNode(m_root, skeletal, c);
	}

	ExpandAll();
}

void WxSkeletalTreeCtrl::ChangeSubjectMgr(const ee0::SubjectMgrPtr& sub_mgr)
{
    if (m_sub_mgr == sub_mgr) {
        return;
    }

    for (auto& m : m_messages) {
        m_sub_mgr->UnregisterObserver(m, this);
    }
    m_sub_mgr = sub_mgr;
    for (auto& m : m_messages) {
        m_sub_mgr->RegisterObserver(m, this);
    }
}

void WxSkeletalTreeCtrl::InsertNode(wxTreeItemId parent, const ::model::SkeletalAnim& skeletal, int child)
{
	auto& nodes = skeletal.GetNodes();
	auto& node = nodes[child];
	auto id = InsertItem(parent, 0, node->name);
    auto item = new Item(child);
    m_items.push_back(item);
	SetItemData(id, item);
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
	MsgHelper::SelectSkeletalJoint(*m_sub_mgr, pdata->m_node_id);
}

void WxSkeletalTreeCtrl::ClearAll()
{
    DeleteAllItems();

    m_root.Unset();
    m_items.clear();
}

}