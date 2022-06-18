#include "headers.h"

#include "GUIFramework.h"
#include "Exceptions/BaseGUIFrameworkException.h"
#include "Utility/Holders/WindowHolder.h"

#include "MainWindow.h"

using namespace std;

#pragma warning(disable: 28251)

int wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
	try
	{
		gui_framework::GUIFramework::initUIThreadId();

		gui_framework::WindowHolder holder(make_unique<simple_server::MainWindow>());
		
		holder.runMainLoop();
	}
	catch (const gui_framework::exceptions::BaseGUIFrameworkException& e)
	{
		using gui_framework::BaseDialogBox;

		BaseDialogBox::createMessageBox(gui_framework::utility::to_wstring(e.what(), CP_ACP), L"Error", BaseDialogBox::messageBoxType::ok);
	}
	catch (const exception& e)
	{
		using gui_framework::BaseDialogBox;

		BaseDialogBox::createMessageBox(gui_framework::utility::to_wstring(e.what(), CP_ACP), L"Error", BaseDialogBox::messageBoxType::ok);
	}

	return 0;
}
