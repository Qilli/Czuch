#pragma once
#include <memory>
#include <string>
#include <type_traits>
#include<functional>
#include<deque>
#include<assert.h>

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

#define NEW(x) new x


typedef unsigned long U64;
typedef unsigned int U32;
typedef int I32;
typedef float F32;
typedef double F64;
typedef unsigned short U16;
typedef short I16;
typedef unsigned char U8;
typedef char I8;
typedef int Guid;
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


#define ENUM_FLAG_OPERATORS(T)                                                                                                                                            \
    inline T operator~ (T a) { return static_cast<T>( ~static_cast<std::underlying_type<T>::type>(a) ); }                                                                       \
    inline T operator| (T a, T b) { return static_cast<T>( static_cast<std::underlying_type<T>::type>(a) | static_cast<std::underlying_type<T>::type>(b) ); }                   \
    inline T operator& (T a, T b) { return static_cast<T>( static_cast<std::underlying_type<T>::type>(a) & static_cast<std::underlying_type<T>::type>(b) ); }                   \
    inline T operator^ (T a, T b) { return static_cast<T>( static_cast<std::underlying_type<T>::type>(a) ^ static_cast<std::underlying_type<T>::type>(b) ); }                   \
    inline T& operator|= (T& a, T b) { return reinterpret_cast<T&>( reinterpret_cast<std::underlying_type<T>::type&>(a) |= static_cast<std::underlying_type<T>::type>(b) ); }   \
    inline T& operator&= (T& a, T b) { return reinterpret_cast<T&>( reinterpret_cast<std::underlying_type<T>::type&>(a) &= static_cast<std::underlying_type<T>::type>(b) ); }   \
    inline T& operator^= (T& a, T b) { return reinterpret_cast<T&>( reinterpret_cast<std::underlying_type<T>::type&>(a) ^= static_cast<std::underlying_type<T>::type>(b) ); }


	struct DeletionQueue
	{
		std::deque<std::function<void()>> deletors;

		void PushFunction(std::function<void()>&& function) {
			deletors.push_back(function);
		}

		void Flush() {
			// reverse iterate the deletion queue to execute all the functions
			for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
				(*it)(); //call functors
			}

			deletors.clear();
		}
	};

	struct WindowInfo
	{
		static U32 Width;
		static U32 Height;
	};


	enum class ValidationMode
	{
		Disabled,	// No validation is enabled
		Enabled,	// CPU command validation
		GPU,		// CPU and GPU-based validation
		Verbose		// Print all warnings, errors and info messages
	};

	enum class EngineMode
	{
		Runtime,
		Editor
	};

	struct RenderSettings
	{
		bool dynamicRendering = false;
		bool offscreenRendering = false;
		bool mainRenderPassActive = true;
		ValidationMode validationMode = ValidationMode::Disabled;
		EngineMode engineMode = EngineMode::Runtime;
	};

}