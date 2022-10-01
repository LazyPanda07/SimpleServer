#include "headers.h"

#include "GUIFramework.h"
#include "Utility/Holders/WindowHolder.h"
#include "Utility.h"

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
	catch (const exception& e)
	{
		simple_server::utility::showError(e);
	}

	return 0;
}
