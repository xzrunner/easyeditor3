#pragma once

#include <painting0/Camera.h>

#include <rapidjson/document.h>

#include <string>

namespace ee0 { class WxStagePage; }

namespace ee3
{

class Serializer
{
public:
	static void StoreToJson(const std::string& filepath, const ee0::WxStagePage* stage);
	static void LoadFroimJson(const std::string& filepath, ee0::WxStagePage* stage);

private:
	static rapidjson::Value StoreCamera(const std::shared_ptr<pt0::Camera>& camera, rapidjson::MemoryPoolAllocator<>& alloc);
	static pt0::CameraPtr LoadCamera(const rapidjson::Value& val);

}; // Serializer

}