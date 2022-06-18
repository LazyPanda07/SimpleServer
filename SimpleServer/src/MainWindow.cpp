#include "MainWindow.h"

using namespace std;

CREATE_DEFAULT_WINDOW_FUNCTION(MainWindow)

namespace simple_server
{
	int MainWindow::getCenterX(int width)
	{
		return (GetSystemMetrics(SM_CXSCREEN) - GetSystemMetrics(SM_CXBORDER) - width) / 2;
	}

	int MainWindow::getCenterY(int height)
	{
		return (GetSystemMetrics(SM_CYSCREEN) - GetSystemMetrics(SM_CYBORDER) - height) / 2;
	}

	MainWindow::MainWindow() :
		SeparateWindow
		(
			L"SimpleServer",
			L"Simple Server",
			gui_framework::utility::ComponentSettings
			(
				MainWindow::getCenterX(800),
				MainWindow::getCenterY(600),
				800,
				600
			),
			"MainWindow"
		)
	{
		setExitMode(exitMode::quit);
	}
}
