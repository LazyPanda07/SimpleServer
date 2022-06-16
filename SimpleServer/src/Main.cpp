#include "headers.h"

#include "GUIFramework.h"
#include "Exceptions/BaseGUIFrameworkException.h"

using namespace std;

#pragma warning(disable: 28251)

int wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
	try
	{
		gui_framework::GUIFramework& instance = gui_framework::GUIFramework::get();
	}
	catch (const gui_framework::exceptions::BaseGUIFrameworkException& e)
	{
		// TODO: show error
	}
	catch (const exception& e)
	{
		// TODO: show error
	}

	return 0;
}
