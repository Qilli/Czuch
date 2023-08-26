#pragma once

#ifdef CZUCH_PLATFORM_WINDOWS
	#ifdef CZUCH_BUILD_DLL
		#define CZUCH_API __declspec(dllexport)
	#else
		#define CZUCH_API __declspec(dllimport)
	#endif
#else
	#error Czuch do not support this platform
#endif