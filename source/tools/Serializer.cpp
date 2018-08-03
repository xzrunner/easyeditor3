#include "ee3/Serializer.h"
#include "ee3/WxStageCanvas.h"

#include <ee0/MsgHelper.h>
#include <ee0/WxStagePage.h>

#include <painting3/OrthoCam.h>
#include <painting3/PerspCam.h>
#ifndef GAME_OBJ_ECS
#include <node0/SceneNode.h>
#endif // GAME_OBJ_ECS
#include <js/RapidJsonHelper.h>
#include <ns/NodeSerializer.h>
#include <guard/check.h>

#include <boost/filesystem.hpp>

namespace ee3
{

void Serializer::StoreToJson(const std::string& filepath, const ee0::WxStagePage* stage)
{
	rapidjson::Document doc;
	doc.SetObject();

	rapidjson::Value val_nodes;
	val_nodes.SetArray();

	auto dir = boost::filesystem::path(filepath).parent_path().string();

	auto& alloc = doc.GetAllocator();
	stage->Traverse([&](const ee0::GameObj& obj)->bool
	{
		rapidjson::Value val_node;
#ifndef GAME_OBJ_ECS
		ns::NodeSerializer::StoreToJson(obj, dir, val_node, alloc);
#endif // GAME_OBJ_ECS
		val_nodes.PushBack(val_node, alloc);
		return true;
	});
	doc.AddMember("nodes", val_nodes, alloc);

	auto canvas = std::dynamic_pointer_cast<const WxStageCanvas>(stage->GetImpl().GetCanvas());
	auto& camera = canvas->GetCamera();
	rapidjson::Value cam_val = StoreCamera(camera, alloc);
	doc.AddMember("camera", cam_val, alloc);

	js::RapidJsonHelper::WriteToFile(filepath.c_str(), doc);
}

void Serializer::LoadFroimJson(const std::string& filepath, ee0::WxStagePage* stage)
{
	rapidjson::Document doc;
	js::RapidJsonHelper::ReadFromFile(filepath.c_str(), doc);

	auto dir = boost::filesystem::path(filepath).parent_path().string();

	auto& nodes_val = doc["nodes"];
	for (auto& node_val : nodes_val.GetArray())
	{
#ifndef GAME_OBJ_ECS
		auto obj = std::make_shared<n0::SceneNode>();
		ns::NodeSerializer::LoadFromJson(obj, dir, node_val);

		bool succ = ee0::MsgHelper::InsertNode(*stage->GetSubjectMgr(), obj);
		GD_ASSERT(succ, "no MSG_INSERT_SCENE_NODE");
#endif // GAME_OBJ_ECS
	}

	auto camera = LoadCamera(doc["camera"]);
	auto canvas = std::dynamic_pointer_cast<const WxStageCanvas>(stage->GetImpl().GetCanvas());
	std::const_pointer_cast<WxStageCanvas>(canvas)->SetCamera(camera);
}

rapidjson::Value Serializer::StoreCamera(const std::shared_ptr<pt0::Camera>& camera, rapidjson::MemoryPoolAllocator<>& alloc)
{
	rapidjson::Value val;
	val.SetObject();

	auto cam_type = camera->TypeID();
	if (cam_type == pt0::GetCamTypeID<pt3::OrthoCam>())
	{
		val.AddMember("type", "ortho", alloc);
	}
	else if (cam_type == pt0::GetCamTypeID<pt3::PerspCam>())
	{
		val.AddMember("type", "perspective", alloc);

		auto persp_cam = std::dynamic_pointer_cast<const pt3::PerspCam>(camera);

		rapidjson::Value pos_val;
		auto& pos = persp_cam->GetPos();
		pos_val.SetObject();
		pos_val.AddMember("x", pos.x, alloc);
		pos_val.AddMember("y", pos.y, alloc);
		pos_val.AddMember("z", pos.z, alloc);
		val.AddMember("pos", pos_val, alloc);

		rapidjson::Value target_val;
		auto& target = persp_cam->GetTarget();
		target_val.SetObject();
		target_val.AddMember("x", target.x, alloc);
		target_val.AddMember("y", target.y, alloc);
		target_val.AddMember("z", target.z, alloc);
		val.AddMember("target", target_val, alloc);

		rapidjson::Value up_val;
		auto& up = persp_cam->GetUpDir();
		up_val.SetObject();
		up_val.AddMember("x", up.x, alloc);
		up_val.AddMember("y", up.y, alloc);
		up_val.AddMember("z", up.z, alloc);
		val.AddMember("up", up_val, alloc);
	}

	return val;
}

pt0::CameraPtr Serializer::LoadCamera(const rapidjson::Value& val)
{
	pt0::CameraPtr ret = nullptr;

	auto type = val["type"].GetString();
	if (type == "ortho")
	{
		ret = std::make_shared<pt3::OrthoCam>(pt3::OrthoCam::VP_ZY);
	}
	else if (type == "perspective")
	{
		auto persp_cam = std::make_shared<pt3::PerspCam>();

		sm::vec3 pos;
		auto& pos_val = val["pos"];
		pos.x = pos_val["x"].GetFloat();
		pos.y = pos_val["y"].GetFloat();
		pos.z = pos_val["z"].GetFloat();

		sm::vec3 target;
		auto& target_val = val["target"];
		target.x = target_val["x"].GetFloat();
		target.y = target_val["y"].GetFloat();
		target.z = target_val["z"].GetFloat();

		sm::vec3 up;
		auto& up_val = val["up"];
		up.x = up_val["x"].GetFloat();
		up.y = up_val["y"].GetFloat();
		up.z = up_val["z"].GetFloat();

		persp_cam->Reset(pos, target, up);

		ret = persp_cam;
	}

	return ret;
}

}