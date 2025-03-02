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
IncludeDir["Vulkan"] = "C:/VulkanSDK/1.3.296.0/Include"
IncludeDir["YamlCpp"] = "Czuch/vendors/yaml-cpp/include"
IncludeDir['Assimp'] = "Czuch/vendors/Assimp/include"
IncludeDir["ImGuizmoCzuch"]= "Czuch/vendors/ImGuizmoCzuch"
IncludeDir["Glslang"]="Czuch/vendors/glslang/include/glslang"

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
		"%{IncludeDir.Assimp}",
		"%{IncludeDir.Glslang}"
	}

	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"yaml-cpp",
		"Czuch/vendors/Assimp/Libs/assimp-vc143-mtd.lib",
		"$(VULKAN_SDK)/lib/vulkan-1.lib",
		"Czuch/vendors/glslang/Debug/glslangd.lib",
		"Czuch/vendors/glslang/Debug/SPIRVd.lib",
		"Czuch/vendors/glslang/Debug/SPIRV-Toolsd.lib",
		"Czuch/vendors/glslang/Debug/SPIRV-Tools-optd.lib",
		"Czuch/vendors/glslang/Debug/glslang-default-resource-limitsd.lib"
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
		"%{IncludeDir.YamlCpp}",
		"%{IncludeDir.Assimp}",
	}

	links
	{
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
			"%{IncludeDir.Assimp}",
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