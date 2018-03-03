#pragma once

#include <rapidjson/document.h>

#include <string>

namespace ee0 { class WxStagePage; }
namespace pt3 { class Camera; }

namespace ee3
{

class Serializer
{
public:
	static void StoreToJson(const std::string& filepath, const ee0::WxStagePage* stage);
	static void LoadFroimJson(const std::string& filepath, ee0::WxStagePage* stage);

private:
	static rapidjson::Value StoreCamera(const pt3::Camera& cam, rapidjson::MemoryPoolAllocator<>& alloc);
	static void LoadCamera(const rapidjson::Value& val, pt3::Camera& cam);

}; // Serializer

}