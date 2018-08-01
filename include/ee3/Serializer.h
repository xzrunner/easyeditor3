#pragma once

#include <painting3/ICamera.h>

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
	static rapidjson::Value StoreCamera(const pt3::ICameraPtr& cam, rapidjson::MemoryPoolAllocator<>& alloc);
	static pt3::ICameraPtr LoadCamera(const rapidjson::Value& val);

}; // Serializer

}