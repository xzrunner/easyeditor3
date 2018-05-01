#include "ee3/WxLibGeoPage.h"

#include <ee0/WxLibraryList.h>
#include <ee0/WxLibraryItem.h>

#include <painting3/AABB.h>
#include <node0/SceneNode.h>
#include <node3/CompModel.h>
#include <node3/CompAABB.h>
#include <model/SurfaceFactory.h>
#include <model/ParametricEquations.h>

namespace ee3
{

WxLibGeoPage::WxLibGeoPage(wxWindow* parent)
	: ee0::WxLibraryPage(parent, "Geometric", true)
{
	LoadDefaultSymbol();
}

void WxLibGeoPage::LoadDefaultSymbol()
{
	LoadGeometric(model::Cone::TYPE_NAME);
	LoadGeometric(model::Sphere::TYPE_NAME);
	LoadGeometric(model::Torus::TYPE_NAME);
	LoadGeometric(model::TrefoilKnot::TYPE_NAME);
	LoadGeometric(model::MobiusStrip::TYPE_NAME);
	LoadGeometric(model::KleinBottle::TYPE_NAME);
}

void WxLibGeoPage::LoadGeometric(const std::string& name)
{
	std::string filepath = name + ".param";
	m_list->Insert(std::make_shared<ee0::WxLibraryItem>(filepath));
}

}