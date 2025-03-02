project "yaml-cpp"
	kind "StaticLib"
	language "C++"
	location "Czuch/vendors/yaml-cpp"

	targetdir("bin/" .. outputdir .. "/%{prj.name}")
	objdir("bin-int/" .. outputdir .. "/%{prj.name}")

	repodir= "Czuch/vendors/yaml-cpp/"

	files
	{
		 repodir.."src/**.h",
		 repodir.."src/**.cpp",
		 repodir.."include/**.h",
	}
	
	includedirs
	{
		repodir.."include"
	}

	filter "system:windows"
		systemversion "latest"
		cppdialect "C++20"
		staticruntime "On"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
		buildoptions "/MDd"
		defines
		{
			"YAML_CPP_STATIC_DEFINE"
		}

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
		buildoptions "/MD"
		defines
		{
			"YAML_CPP_STATIC_DEFINE"
		}

	