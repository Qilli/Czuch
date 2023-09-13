workspace "Czuch"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

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
		"%{prj.name}/vendors/spdlog/include"
	}

	filter "system:windows"

		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"CZUCH_PLATFORM_WINDOWS",
			"CZUCH_BUILD_DLL"
		}

		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Playground")
		}

	filter "configurations:Debug"
		defines
		{
			"CZUCH_DEBUG"
		}
		optimize "On"

	filter "configurations:Release"
		defines
		{
			"CZUCH_RELEASE"
		}
		optimize "On"

	filter "configurations:Dist"
		defines
		{
			"CZUCH_DIST"
		}
		optimize "On"

project "Playground"
	
	location "Playground"
	kind "ConsoleApp"
	language "C++"

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
		"Czuch/source"
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

	filter "configurations:Release"
		defines
		{
			"CZUCH_RELEASE"
		}
		optimize "On"

	filter "configurations:Dist"
		defines
		{
			"CZUCH_DIST"
		}
		optimize "On"