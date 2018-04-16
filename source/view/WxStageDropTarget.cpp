#include "ee3/WxStageDropTarget.h"
#include "ee3/NodeFactory.h"
#include "ee3/WxStageCanvas.h"

#include <ee0/WxLibraryPanel.h>
#include <ee0/MessageID.h>
#include <ee0/VariantSet.h>
#include <ee0/WxLibraryItem.h>
#include <ee0/MsgHelper.h>
#include <ee0/WxStagePage.h>
#include <ee0/SubjectMgr.h>

#include <guard/check.h>
#ifndef GAME_OBJ_ECS
#include <node0/SceneNode.h>
#include <node3/CompTransform.h>
#include <ns/NodeFactory.h>
#else
#include <es/EntityFactory.h>
#endif // GAME_OBJ_ECS
#include <sx/StringHelper.h>

namespace ee3
{

WxStageDropTarget::WxStageDropTarget(ECS_WORLD_PARAM ee0::WxLibraryPanel* library, ee0::WxStagePage* stage)
	: m_library(library)
	, m_stage(stage)
	ECS_WORLD_SELF_ASSIGN
{
}

void WxStageDropTarget::OnDropText(wxCoord x, wxCoord y, const wxString& text)
{
	std::vector<std::string> keys;
	sx::StringHelper::Split(text.ToStdString().c_str(), ",", keys);

	if (keys.size() <= 1) {
		return;
	}

	for (int i = 1, n = keys.size(); i < n; ++i)
	{
		int idx = std::stoi(keys[i].c_str());
		auto item = m_library->GetItem(idx);
		if (!item) {
			continue;
		}

#ifndef GAME_OBJ_ECS
		auto obj = ns::NodeFactory::Create(item->GetFilepath());
		if (!obj) {
			continue;
		}
#else
		auto obj = es::EntityFactory::Create(m_world, item->GetFilepath());
#endif // GAME_OBJ_ECS

		InsertNode(obj);

#ifndef GAME_OBJ_ECS
		// transform
		sm::vec3 pos = TransPosScrToProj3d(x, y);
		auto& ctrans = obj->AddUniqueComp<n3::CompTransform>();
		// todo
		//auto parent = obj->GetParent();
		//if (parent) {
		//	auto p_pos = parent->GetUniqueComp<n3::CompTransform>().GetTransformMat() * sm::vec3(0, 0, 0);
		//	pos -= p_pos;
		//}
		ctrans.SetPosition(pos);
#else
		
#endif // GAME_OBJ_ECS
	}

	m_stage->GetSubjectMgr()->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
}

void WxStageDropTarget::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames)
{
}

void WxStageDropTarget::InsertNode(ee0::GameObj& obj)
{
	bool succ = ee0::MsgHelper::InsertNode(*m_stage->GetSubjectMgr(), obj);
	GD_ASSERT(succ, "no MSG_INSERT_SCENE_NODE");
}

sm::vec3 WxStageDropTarget::TransPosScrToProj3d(int x, int y) const
{
	auto& canvas = std::dynamic_pointer_cast<const WxStageCanvas>(m_stage->GetImpl().GetCanvas());
	if (!canvas) {
		return sm::vec3();
	}

	auto& vp = canvas->GetViewport();
	auto& cam = canvas->GetCamera();
	auto dir = vp.TransPos3ScreenToDir(sm::vec2(x, y), cam);
	sm::vec3 ret = dir.Normalized() * cam.GetDistance();
	ret.y = 0;
	return ret;
}

}