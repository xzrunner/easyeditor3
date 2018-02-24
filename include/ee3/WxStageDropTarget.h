#pragma once

#include <ee0/WxDropTarget.h>

#include <node0/typedef.h>

namespace ee0 { class WxLibraryPanel; }

namespace ee3
{

class WxStagePage;

class WxStageDropTarget : public ee0::WxDropTarget
{
public:
	WxStageDropTarget(ee0::WxLibraryPanel* library, WxStagePage* stage);

	virtual void OnDropText(wxCoord x, wxCoord y, const wxString& text) override;
	virtual void OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames) override;

private:
	void InsertNode(n0::SceneNodePtr& node);

private:
	ee0::WxLibraryPanel* m_library;
	WxStagePage*           m_stage;

}; // WxStageDropTarget

}