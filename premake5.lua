workspace "Czuch"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["GLFW"] = "Czuch/vendors/GLFW/include"
IncludeDir["Glad"] = "Czuch/vendors/Glad/include"
IncludeDir["ImGui"] = "Czuch/vendors/ImGui"
IncludeDir["Vulkan"] = "F:/VulkanSDK/Include"

include "premake-glfw.lua"
include "premake-imgui.lua"
include "Czuch/vendors/Glad/premake-glad.lua"


project "Czuch"

	location "Czuch"
	kind "SharedLib"
	language "C++"

	targetdir("bin/" .. outputdir .. "/%{prj.name}")
	objdir("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "czpch.h"
	pchsource "Czuch/source/czpch.cpp"

	files
	{
		"%{prj.name}/source/**.h",
		"%{prj.name}/source/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/source",
		"%{prj.name}/vendors/spdlog/include",
		"%{prj.name}/vendors/glm",
		"%{prj.name}/vendors/vma",
		"%{prj.name}/vendors/stb",
		"%{prj.name}/vendors/entt",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.Vulkan}"
	}

	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"$(VULKAN_SDK)/lib/vulkan-1.lib"
	}

	filter "system:windows"

		cppdialect "C++20"
		staticruntime "Off"
		systemversion "latest"

		defines
		{
			"CZUCH_PLATFORM_WINDOWS",
			"CZUCH_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
		}

		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Playground"),
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/CzuchEditor")
		}

	filter "configurations:Debug"
		defines
		{
			"CZUCH_DEBUG",
			"CZUCH_ENABLE_ASSERTS"
		}
		optimize "On"
		buildoptions "/MDd"

	filter "configurations:Release"
		defines
		{
			"CZUCH_RELEASE"
		}
		optimize "On"
		buildoptions "/MD"

	filter "configurations:Dist"
		defines
		{
			"CZUCH_DIST"
		}
		optimize "On"
		buildoptions "/MD"

project "Playground"
	
	location "Playground"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"

	targetdir("bin/" .. outputdir .. "/%{prj.name}")
	objdir("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/source/**.h",
		"%{prj.name}/source/**.cpp"
	}

	includedirs
	{
		"Czuch/vendors/spdlog/include",
		"Czuch/vendors/glm",
		"Czuch/vendors/entt",
		"Czuch/source",
	}

	links
	{
		"Czuch"
	}

	defines
	{
		"CZUCH_PLATFORM_WINDOWS"
	}

	filter "configurations:Debug"
		defines
		{
			"CZUCH_DEBUG"
		}
		optimize "On"
		buildoptions "/MDd"

	filter "configurations:Release"
		defines
		{
			"CZUCH_RELEASE"
		}
		optimize "On"
		buildoptions "/MD"

	filter "configurations:Dist"
		defines
		{
			"CZUCH_DIST"
		}
		optimize "On"
		buildoptions "/MD"


		project "CzuchEditor"
	
		location "CzuchEditor"
		kind "ConsoleApp"
		language "C++"
		cppdialect "C++20"
	
		targetdir("bin/" .. outputdir .. "/%{prj.name}")
		objdir("bin-int/" .. outputdir .. "/%{prj.name}")
	
		files
		{
			"%{prj.name}/source/**.h",
			"%{prj.name}/source/**.cpp"
		}
	
		includedirs
		{
			"Czuch/vendors/spdlog/include",
			"Czuch/vendors/glm",
			"Czuch/vendors/entt",
			"Czuch/source",
			"%{IncludeDir.ImGui}",
		}
	
		links
		{
			"ImGui",
			"Czuch"
		}
	
		defines
		{
			"CZUCH_PLATFORM_WINDOWS"
		}
	
		filter "configurations:Debug"
			defines
			{
				"CZUCH_DEBUG"
			}
			optimize "On"
			buildoptions "/MDd"
	
		filter "configurations:Release"
			defines
			{
				"CZUCH_RELEASE"
			}
			optimize "On"
			buildoptions "/MD"
	
		filter "configurations:Dist"
			defines
			{
				"CZUCH_DIST"
			}
			optimize "On"
			buildoptions "/MD"