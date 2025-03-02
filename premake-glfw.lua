project "GLFW"
	kind "StaticLib"
	language "C"
	location "Czuch/vendors/GLFW"

	targetdir("bin/" .. outputdir .. "/%{prj.name}")
	objdir("bin-int/" .. outputdir .. "/%{prj.name}")

	repodir= "Czuch/vendors/GLFW/"

	files
	{
		repodir .. "include/GLFW/glfw3.h",
		repodir .. "include/GLFW/glfw3native.h",
		repodir .."src/internal.h",
		repodir .."src/platform.h",
		repodir .."src/mappings.h",
		repodir .."src/context.c",
		repodir .."src/init.c",
		repodir .."src/input.c",
		repodir .."src/monitor.c",
		repodir .."src/platform.c",
		repodir .."src/vulkan.c",
		repodir .."src/window.c",
		repodir .."src/egl_context.c",
		repodir .."src/osmesa_context.c",
		repodir .."src/null_platform.h",
		repodir .."src/null_joystick.h",
		repodir .."src/null_init.c",
		repodir .."src/null_monitor.c",
		repodir .."src/null_window.c",
		repodir .."src/null_joystick.c"
		 
	}

	filter "system:windows"
		systemversion "latest"
		staticruntime "On"

		
		files
		{
		repodir .."src/win32_init.c",
		repodir .."src/win32_module.c",
		repodir .."src/win32_joystick.c",
		repodir .."src/win32_monitor.c",
		repodir .."src/win32_time.h",
		repodir .."src/win32_time.c",
		repodir .."src/win32_thread.h",
		repodir .."src/win32_thread.c",
		repodir .."src/win32_window.c",
		repodir .."src/wgl_context.c",
		repodir .."src/egl_context.c",
		repodir .."src/osmesa_context.c"
		}

		defines
		{
			"_GLFW_WIN32",
			"_CRT_SECURE_NO_WARNINGS"
		}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"