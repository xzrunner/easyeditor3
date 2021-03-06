#pragma once

#include <ee0/Observer.h>
#include <ee0/typedef.h>

#include <wx/treectrl.h>

namespace model { class SkeletalAnim; }

namespace ee3
{

class WxSkeletalTreeCtrl : public wxTreeCtrl, public ee0::Observer
{
public:
	WxSkeletalTreeCtrl(wxWindow* parent, const ee0::SubjectMgrPtr& sub_mgr);
	virtual ~WxSkeletalTreeCtrl();

	virtual void OnNotify(uint32_t msg, const ee0::VariantSet& variants) override;

	void LoadFromSkeletal(const ::model::SkeletalAnim& skeletal);

    void ChangeSubjectMgr(const ee0::SubjectMgrPtr& sub_mgr);

private:
	void InsertNode(wxTreeItemId parent,
		const ::model::SkeletalAnim& skeletal, int child);

	void OnSelChanged(wxTreeEvent& event);

    void ClearAll();

private:
	class Item : public wxTreeItemData
	{
	public:
		Item(int node_id) : m_node_id(node_id) {}

	public:
		int m_node_id;

	}; // Item

private:
	ee0::SubjectMgrPtr m_sub_mgr;

    std::vector<int> m_messages;

	wxTreeItemId       m_root;
	std::vector<Item*> m_items;

	DECLARE_EVENT_TABLE()

}; // WxSkeletalTreeCtrl

}