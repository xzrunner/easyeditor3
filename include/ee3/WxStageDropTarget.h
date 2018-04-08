#pragma once

#include <ee0/WxDropTarget.h>
#include <ee0/GameObj.h>

namespace ee0 { class WxLibraryPanel; class WxStagePage;  }

namespace ee3
{

class WxStageDropTarget : public ee0::WxDropTarget
{
public:
	WxStageDropTarget(ee0::WxLibraryPanel* library, ee0::WxStagePage* stage);

	virtual void OnDropText(wxCoord x, wxCoord y, const wxString& text) override;
	virtual void OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames) override;

private:
	void InsertNode(ee0::GameObj& obj);

	sm::vec3 TransPosScrToProj3d(int x, int y) const;
	
private:
	ee0::WxLibraryPanel* m_library;
	ee0::WxStagePage*    m_stage;

}; // WxStageDropTarget

}