#pragma once

#include <ee0/WxDropTarget.h>
#include <ee0/GameObj.h>

namespace ee0 { class WxLibraryPanel; class WxStagePage;  }
ECS_WORLD_DECL

namespace ee3
{

class WxStageDropTarget : public ee0::WxDropTarget
{
public:
	WxStageDropTarget(ECS_WORLD_PARAM ee0::WxLibraryPanel* library, ee0::WxStagePage* stage);

	virtual void OnDropText(wxCoord x, wxCoord y, const wxString& text) override;
	virtual void OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames) override;

private:
	void InsertNode(ee0::GameObj& obj);

	void InitNodeComp(const ee0::GameObj& obj, const sm::vec3& pos,
		const std::string& filepath);

	sm::vec3 TransPosScrToProj3d(int x, int y) const;

private:
	ee0::WxLibraryPanel* m_library;
	ee0::WxStagePage*    m_stage;
	ECS_WORLD_SELF_DEF

}; // WxStageDropTarget

}