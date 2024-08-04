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
IncludeDir["YamlCpp"] = "Czuch/vendors/yaml-cpp/include"
IncludeDir["ImGuizmoCzuch"]= "Czuch/vendors/ImGuizmoCzuch"

include "premake-glfw.lua"
include "premake-imgui.lua"
include "premake-yaml-cpp.lua"
include "Czuch/vendors/Glad/premake-glad.lua"


project "Czuch"

	location "Czuch"
	kind "SharedLib"
	language "C++"

	targetdir("bin/" .. outputdir .. "/%{prj.name}")
	objdir("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "czpch.h"
	pchsource "Czuch/source/czpch.cpp"

	filter "files:vendors/ImGuizmoCzuch/**.cpp"
	flags { "NoPCH" }
	filter {}

	files
	{
		"%{prj.name}/source/**.h",
		"%{prj.name}/source/**.cpp",
		"%{prj.name}/vendors/ImGuizmoCzuch/ImGuizmo.h",
		"%{prj.name}/vendors/ImGuizmoCzuch/ImGuizmo.cpp"
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
		"%{IncludeDir.Vulkan}",
		"%{IncludeDir.YamlCpp}",
		"%{IncludeDir.ImGuizmoCzuch}",
	}

	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"yaml-cpp",
		"$(VULKAN_SDK)/lib/vulkan-1.lib"
	}

	filter "files:vendors/ImGuizmoCzuch/**.cpp"
	flags { "NoPCH" }
	filter {}

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


		filter "files:vendors/ImGuizmoCzuch/**.cpp"
		flags { "NoPCH" }
		filter {}
	

	
		files
		{
			"%{prj.name}/source/**.h",
			"%{prj.name}/source/**.cpp",
			"%{IncludeDir.ImGuizmoCzuch}/ImGuizmo.h",
			"%{IncludeDir.ImGuizmoCzuch}/ImGuizmo.cpp"
		}
	
		includedirs
		{
			"Czuch/vendors/spdlog/include",
			"Czuch/vendors/glm",
			"Czuch/vendors/entt",
			"Czuch/source",
			"%{IncludeDir.ImGui}",
			"%{IncludeDir.YamlCpp}",
			"%{IncludeDir.ImGuizmoCzuch}",
		}
	
		links
		{
			"ImGui",
			"yaml-cpp",
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