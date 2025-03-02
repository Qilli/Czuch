#pragma once
#include<string>

namespace Czuch
{
	class CZUCH_API WinUtils
	{
	public:
		static std::string GetOpenFileNameDialog(const char* filter = "All Files\0*.*\0", const char* title = "Open File");
		static std::string GetSaveFileNameDialog(const char* filter = "All Files\0*.*\0", const char* title = "Save File");
		static bool ShowYesNoDialog(const char* title, const char* message);
	};
}