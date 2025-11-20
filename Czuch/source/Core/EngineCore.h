#pragma once
#include <type_traits>
#include<functional>
#include<deque>
#include<assert.h>
#include<memory>
#include <tuple>
#include"Common.h"


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

namespace Czuch
{

	#define Invalid_Handle_Id -1
	#define InvalidID 0

	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScoped(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;

	template<typename T, typename ... Args>
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


#define HAS_FLAG(flags, flag) ((flags & flag) == flag)

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

	struct DebugSettings
	{
	public:
		DebugSettings()
			: enableDebugDrawOBBForMeshes(false)
			, debugDrawOBBForMeshesScale(1.01f)
		{};
		bool IsDebugDrawOBBForMeshesEnabled() const
		{
			return enableDebugDrawOBBForMeshes;
		}
		void SetDebugDrawOBBForMeshesEnabled(bool enabled)
		{
			enableDebugDrawOBBForMeshes = enabled;
		}
		float GetDebugDrawOBBForMeshesScale() const
		{
			return debugDrawOBBForMeshesScale;
		}
		void SetDebugDrawOBBForMeshesScale(float scale)
		{
			debugDrawOBBForMeshesScale = scale;
		}

		bool IsDebugDrawNormalForMeshesEnabled() const
		{
			return debugDrawNormalForMeshes;
		}

		void SetDebugDrawNormalForMeshesEnabled(bool enabled)
		{
			debugDrawNormalForMeshes = enabled;
		}

		void SetDebugDrawSelectedEntityID(intptr_t entityID)
		{
			m_DebugDrawSelectedEntityID = entityID;
		}

		intptr_t GetDebugDrawSelectedEntityID() const
		{
			return m_DebugDrawSelectedEntityID;
		}

	private:
		intptr_t m_DebugDrawSelectedEntityID; // Entity used for debug drawing, if needed
		bool debugDrawNormalForMeshes = false; // Enable or disable debug drawing of normals for meshes
		bool enableDebugDrawOBBForMeshes; // Enable or disable debug drawing of OBB for meshes
		float debugDrawOBBForMeshesScale = 1.01f; //Scale for debug drawing OBB for meshes 1.0 means it will fit exactly the mesh 1.1 will add 10% in size
	};

	struct EngineSettings
	{
		bool dynamicRendering = false;
		ValidationMode validationMode = ValidationMode::Disabled;
		EngineMode engineMode = EngineMode::Runtime;
		U32 targetWidth = 0;//only for dynamic size rendering
		U32 targetHeight = 0;
		std::string startPath = "Czuch";
		DebugSettings debugSettings;
		U32 directionalShadowMapResolution = 1024.0f;

		const CzuchStr& GetStartPath() const
		{
			return startPath;
		}

		bool RenderingTargetSizeExternallySet() const
		{
			return EngineMode::Editor == engineMode;
		}
	};

}