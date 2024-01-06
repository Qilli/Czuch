#pragma once
#include <memory>
#include <string>

#ifdef CZUCH_PLATFORM_WINDOWS
	#ifdef CZUCH_BUILD_DLL
		#define CZUCH_API __declspec(dllexport)
	#else
		#define CZUCH_API __declspec(dllimport)
	#endif
#else
	#error Czuch do not support this platform
#endif

#ifdef CZUCH_ENABLE_ASSERTS
#define CZUCH_BE_ASSERT(x,...) if(!(x)){LOG_BE_ERROR("Assertion failed: {0}",__VA_ARGS__); __debugbreak();}
#define CZUCH_ASSERT(x,...) if(!(x)){LOG_FE_ERROR("Assertion failed: {0}",__VA_ARGS__); __debugbreak();}
#else
#define CZUCH_BE_ASSERT(x,...)
#define CZUCH_ASSERT(x,...)
#endif


typedef unsigned long U64;
typedef unsigned int U32;
typedef int I32;
typedef float F32;
typedef double F64;
typedef unsigned short U16;
typedef short I16;
typedef unsigned char U8;
typedef char I8;
typedef unsigned int Guid;
typedef std::string CzuchStr;

const CzuchStr EMPTY_STRING = "";

namespace Czuch
{
	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScoped(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;

	template<typename T,typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

}