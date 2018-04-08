#include "ee3/NodeFactory.h"

#include <ee0/CompNodeEditor.h>

//#include <sprite2/SymType.h>
//#include <sprite2/ModelSymbol.h>
//#include <sprite2/SNodeSymbol.h>
#include <node0/SceneNode.h>
#include <node3/CompModel.h>
#include <node3/CompAABB.h>

namespace ee3
{

CU_SINGLETON_DEFINITION(NodeFactory);

NodeFactory::NodeFactory()
{
}

//ee0::GameObj NodeFactory::Create(const s2::SymPtr& sym)
//{
//	if (!sym) {
//		return nullptr;
//	}
//
//	ee0::GameObj obj = nullptr;
//
//	switch (sym->Type())
//	{
//	case s2::SYM_MODEL:
//		{
//			auto model_sym = std::dynamic_pointer_cast<s2::ModelSymbol>(sym);
//			auto& model = model_sym->GetModel();
//			if (model)
//			{
//				obj = std::make_shared<n0::SceneNode>();
//
//				// model
//				auto& cmodel = obj->AddSharedComp<n3::CompModel>();
//				cmodel.SetModel(model);
//
//				// aabb
//				auto& caabb = obj->AddUniqueComp<n3::CompAABB>();
//				caabb.SetAABB(model_sym->GetAABB());
//
//				// editor
//				obj->AddUniqueComp<ee0::CompNodeEditor>();
//			}
//		}
//		break;
//	case s2::SYM_SNODE:
//		{
//			auto snode_sym = std::dynamic_pointer_cast<s2::SNodeSymbol>(sym);
//			auto& src_node = snode_sym->GetNode();
//			if (src_node)
//			{
//				obj = std::make_shared<n0::SceneNode>();
//
//				// model
//				auto& cmodel = obj->AddSharedComp<n3::CompModel>();
//				auto& src_model = src_node->GetSharedComp<n3::CompModel>();
//				cmodel.SetModel(src_model.GetModel());
//
//				// aabb
//				auto& caabb = obj->AddUniqueComp<n3::CompAABB>();
//				auto& src_aabb = src_node->GetUniqueComp<n3::CompAABB>();
//				caabb.SetAABB(src_aabb.GetAABB());
//
//				// editor
//				obj->AddUniqueComp<ee0::CompNodeEditor>();
//			}
//		}
//		break;
//	}
//
//	return obj;
//}

}