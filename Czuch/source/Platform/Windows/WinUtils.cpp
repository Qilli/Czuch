#include"czpch.h"
#include"WinUtils.h"
#include<commdlg.h>
#include<WinUser.h>
#include"GLFW/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include"GLFW/glfw3native.h"
#include"EngineRoot.h"

namespace Czuch
{
	std::string WinUtils::GetOpenFileNameDialog(const char* filter, const char* title)
	{
		OPENFILENAMEA ofn;
		char szFile[260] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
		HWND hwnd = glfwGetWin32Window((GLFWwindow*)EngineRoot::GetPtr()->GetWindow().GetNativeWindowPtr());
		ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.hwndOwner = hwnd;
		ofn.lpstrFile = szFile;
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrTitle = title;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		if (GetOpenFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}
		return "";
	}
	std::string WinUtils::GetSaveFileNameDialog(const char* filter, const char* title)
	{
		OPENFILENAMEA ofn;
		char szFile[260] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
		HWND hwnd = glfwGetWin32Window((GLFWwindow*)EngineRoot::GetPtr()->GetWindow().GetNativeWindowPtr());
		ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.hwndOwner = hwnd;
		ofn.lpstrFile = szFile;
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrTitle = title;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		if (GetSaveFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}
		return "";
	}
	bool WinUtils::ShowYesNoDialog(const char* title, const char* message)
	{
		int msgboxID = MessageBoxA(
			NULL,
			message,
			title,
			MB_ICONEXCLAMATION | MB_YESNO
		);

		if (msgboxID == IDYES)
		{
			return true;
		}

		return false;
	}
}