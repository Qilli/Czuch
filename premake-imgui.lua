project "Imgui"
	kind "StaticLib"
	language "C"
	location "Czuch/vendors/ImGui"

	targetdir("bin/" .. outputdir .. "/%{prj.name}")
	objdir("bin-int/" .. outputdir .. "/%{prj.name}")

	repodir= "Czuch/vendors/imgui/"

	files
	{
		repodir .. "imconfig.h",
		repodir .. "imgui.h",
		repodir .."imgui.cpp",
		repodir .."imgui_draw.cpp",
		repodir .."imgui_internal.h",
		repodir .."imgui_widgets.cpp",
		repodir .."imstb_rectpack.h",
		repodir .."imstb_textedit.h",
		repodir .."imstb_truetype",
		repodir .."imgui_demo.cpp", 
	}

	filter "system:windows"
		systemversion "latest"
		cppdialect "C++17"
		staticruntime "On"

	