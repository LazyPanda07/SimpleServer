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

	void MainWindow::createMarkup()
	{
		serverButton = new gui_framework::Button(L"Start", L"Start server", gui_framework::utility::ComponentSettings(10, 10, 200, 30), this, bind(&MainWindow::changeServerState, this));

		serverButton->setBackgroundColor(255, 0, 0);
	}

	void MainWindow::changeServerState()
	{
		serverState ?
			this->stopServer() :
			this->startServer();

		serverState = !serverState;
	}

	void MainWindow::startServer()
	{
		if (!serverFolder)
		{
			if (serverFolderDialog->Show(getHandle()) != S_OK)
			{
				return;
			}

			if (serverFolderDialog->GetResult(&serverFolder) != S_OK)
			{
				serverFolder = nullptr;

				return;
			}

			PWSTR pathToFolder;

			serverFolder->GetDisplayName(SIGDN::SIGDN_DESKTOPABSOLUTEEDITING, &pathToFolder);

			server = make_unique<framework::WebFramework>(filesystem::path(pathToFolder) / "server_configuration.json");
		}
		
		server->startServer();

		serverButton->setText(L"Stop server");

		serverButton->setBackgroundColor(0, 255, 0);
	}

	void MainWindow::stopServer()
	{
		serverButton->setText(L"Stopping server...");

		serverButton->setBackgroundColor(128, 128, 128);

		// TODO: disable button

		thread([this]()
			{
				server->stopServer();

				gui_framework::GUIFramework::runOnUIThread([this]()
					{
						serverButton->setText(L"Start server");

						serverButton->setBackgroundColor(255, 0, 0);

						// TODO: enable button
					});
			}).detach();
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
		),
		serverState(false),
		serverFolder(nullptr)
	{
		setExitMode(exitMode::quit);

		if (CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&serverFolder)) != S_OK)
		{
			throw runtime_error("Error");
		}

		this->createMarkup();

		serverFolderDialog->SetOptions(FOS_PICKFOLDERS | FOS_PATHMUSTEXIST);

		IShellItem* folder;

		SHCreateItemFromParsingName(filesystem::current_path().wstring().data(), nullptr, IID_PPV_ARGS(&folder));

		serverFolderDialog->SetFolder(folder);

		folder->Release();
	}
}
