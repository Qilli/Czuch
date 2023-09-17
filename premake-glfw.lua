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
		 repodir .. "src/glfw_config.h",
		 repodir .. "src/context.c",
		 repodir .. "src/init.c",
		 repodir .. "src/input.c",
		 repodir .. "src/monitor.c",
		 repodir .. "src/vulkan.c",
		 repodir .. "src/window.c"
	}

	filter "system:windows"
		systemversion "latest"
		staticruntime "On"

		
		files
		{
		   repodir .. "src/win32_init.c",
		   repodir .. "src/win32_joystick.c",
		   repodir .. "src/win32_monitor.c",
		   repodir .. "src/win32_time.c",
		   repodir .. "src/win32_thread.c",
		   repodir .. "src/win32_window.c",
		   repodir .. "src/wgl_context.c",
		   repodir .. "src/egl_context.c",
		   repodir .. "src/osmesa_context.c"
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