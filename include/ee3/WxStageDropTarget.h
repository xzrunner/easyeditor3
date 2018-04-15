#pragma once

#include <ee0/WxDropTarget.h>
#include <ee0/GameObj.h>

namespace ee0 { class WxLibraryPanel; class WxStagePage;  }
#ifdef GAME_OBJ_ECS
namespace ecsx { class World; }
#endif // GAME_OBJ_ECS

namespace ee3
{

class WxStageDropTarget : public ee0::WxDropTarget
{
public:
	WxStageDropTarget(
#ifdef GAME_OBJ_ECS
		ecsx::World& world,
#endif // GAME_OBJ_ECS
		ee0::WxLibraryPanel* library, 
		ee0::WxStagePage* stage
	);

	virtual void OnDropText(wxCoord x, wxCoord y, const wxString& text) override;
	virtual void OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames) override;

private:
	void InsertNode(ee0::GameObj& obj);

	sm::vec3 TransPosScrToProj3d(int x, int y) const;
	
private:
	ee0::WxLibraryPanel* m_library;
	ee0::WxStagePage*    m_stage;
#ifdef GAME_OBJ_ECS
	ecsx::World&         m_world;
#endif // GAME_OBJ_ECS

}; // WxStageDropTarget

}