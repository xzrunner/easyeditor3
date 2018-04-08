#pragma once

#include <ee0/GameObj.h>

//#include <sprite2/typedef.h>

#include <cu/cu_macro.h>

namespace ee3
{

class NodeFactory
{
public:
	//ee0::GameObj Create(const s2::SymPtr& sym);

	CU_SINGLETON_DECLARATION(NodeFactory);

}; // NodeFactory

}