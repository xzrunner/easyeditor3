#include "ee3/WxStageDropTarget.h"
#include "ee3/WxStageCanvas.h"

#include <ee0/WxLibraryPanel.h>
#include <ee0/MessageID.h>
#include <ee0/VariantSet.h>
#include <ee0/WxLibraryItem.h>
#include <ee0/MsgHelper.h>
#include <ee0/WxStagePage.h>
#include <ee0/SubjectMgr.h>

#include <guard/check.h>
#include <painting3/PerspCam.h>
#ifndef GAME_OBJ_ECS
#include <node0/SceneNode.h>
#include <node3/CompTransform.h>
#include <ns/NodeFactory.h>
#else
#include <es/EntityFactory.h>
#endif // GAME_OBJ_ECS
#include <cpputil/StringHelper.h>

#include <boost/filesystem.hpp>

namespace ee3
{

WxStageDropTarget::WxStageDropTarget(const ur::Device& dev, ECS_WORLD_PARAM
                                     ee0::WxLibraryPanel* library, ee0::WxStagePage* stage)
	: m_dev(dev)
    , m_library(library)
	, m_stage(stage)
	ECS_WORLD_SELF_ASSIGN
{
}

void WxStageDropTarget::OnDropText(wxCoord x, wxCoord y, const wxString& text)
{
	std::vector<std::string> keys;
	cpputil::StringHelper::Split(text.ToStdString().c_str(), ",", keys);

	if (keys.size() <= 1) {
		return;
	}

	sm::vec3 pos = TransPosScrToProj3d(x, y);
	for (int i = 1, n = keys.size(); i < n; ++i)
	{
		int idx = std::stoi(keys[i].c_str());
		auto item = m_library->GetItem(idx);
		if (!item) {
			continue;
		}

#ifndef GAME_OBJ_ECS
		auto obj = ns::NodeFactory::Create(m_dev, item->GetFilepath());
		if (!obj) {
			continue;
		}
#else
		auto obj = es::EntityFactory::Create(m_world, item->GetFilepath());
#endif // GAME_OBJ_ECS

		InitNodeComp(obj, pos, item->GetFilepath());

		InsertNode(obj);
	}

	m_stage->GetSubjectMgr()->NotifyObservers(ee0::MSG_SET_CANVAS_DIRTY);
}

void WxStageDropTarget::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames)
{
}

void WxStageDropTarget::InsertNode(ee0::GameObj& obj)
{
	auto& sub_mgr = *m_stage->GetSubjectMgr();

	bool succ = ee0::MsgHelper::InsertNode(sub_mgr, obj);
	GD_ASSERT(succ, "no MSG_SCENE_NODE_INSERT");

	ee0::MsgHelper::SetEditorDirty(sub_mgr, true);
}

void WxStageDropTarget::InitNodeComp(const ee0::GameObj& obj,
	                                 const sm::vec3& pos,
	                                 const std::string& filepath)
{
	// transform
#ifndef GAME_OBJ_ECS
	auto ext = boost::filesystem::extension(filepath);
	std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
	if (ext == ".bsp")
	{
		auto& ctrans = obj->GetUniqueComp<n3::CompTransform>();
		ctrans.Rotate(sm::Quaternion::CreateFromEulerAngle(0, - SM_PI * 0.5f, 0));
		ctrans.Translate(sm::vec3(0, 10, 0));
	}
	else
	{
		auto& ctrans = obj->GetUniqueComp<n3::CompTransform>();
		// todo
		//auto parent = obj->GetParent();
		//if (parent) {
		//	auto p_pos = parent->GetUniqueComp<n2::CompTransform>().GetTrans().GetMatrix() * sm::vec2(0, 0);
		//	ctrans.SetPosition(obj, pos - p_pos);
		//} else {
		//	ctrans.SetPosition(obj, pos);
		//}
		ctrans.SetPosition(pos);
	}
#else
	e2::SysTransform::SetPosition(m_world, obj, pos);
#endif // GAME_OBJ_ECS
}

sm::vec3 WxStageDropTarget::TransPosScrToProj3d(int x, int y) const
{
	auto& canvas = std::dynamic_pointer_cast<const WxStageCanvas>(m_stage->GetImpl().GetCanvas());
	if (!canvas) {
		return sm::vec3();
	}

	auto& vp = canvas->GetViewport();
	auto& camera = canvas->GetCamera();
	if (camera->TypeID() == pt0::GetCamTypeID<pt3::PerspCam>())
	{
		auto persp_cam = std::dynamic_pointer_cast<pt3::PerspCam>(camera);
		auto dir = vp.TransPos3ScreenToDir(sm::vec2(x, y), *persp_cam);
		sm::vec3 ret = dir * persp_cam->GetDistance();
		ret.y = 0;
		return ret;
	}
	else
	{
		// todo
		return sm::vec3();
	}
}

}