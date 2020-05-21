#pragma once
namespace core
{

#define SINGLETON_DEFINE(TypeName)				\
static TypeName* GetI()					\
{												\
	static TypeName type_instance;				\
	return &type_instance;						\
}												\
												\
TypeName(const TypeName&) = delete;				\
TypeName& operator=(const TypeName&) = delete;	\
TypeName(TypeName&&) = delete;					\
TypeName& operator=(TypeName&&) = delete	

}
