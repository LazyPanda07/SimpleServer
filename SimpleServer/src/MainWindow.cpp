#include "MainWindow.h"

#include <fstream>

#include "MenuItems/MenuItem.h"
#include "Utility.h"
#include "SimpleServerConstants.h"

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
		localization::WTextLocalization& localization = localization::WTextLocalization::get();

		serverButton = new gui_framework::Button(L"Start", localization[constants::localization_keys::startApplicationKey], gui_framework::utility::ComponentSettings(10, 10, 200, 30), this, bind(&MainWindow::changeServerState, this));

		serverButton->setBackgroundColor(255, 0, 0);

		unique_ptr<gui_framework::Menu>& menu = this->createMainMenu(L"Menu");

		menu->addMenuItem(make_unique<gui_framework::MenuItem>(localization[constants::localization_keys::chooseApplicationFolderKey], [this]()
			{
				if (serverFolderDialog->Show(nullptr) != S_OK)
				{
					return;
				}

				if (serverFolderDialog->GetResult(&serverFolder) != S_OK)
				{
					serverFolder = nullptr;

					return;
				}

				PWSTR pathToFolder;

				if (serverFolder->GetDisplayName(SIGDN::SIGDN_DESKTOPABSOLUTEEDITING, &pathToFolder) == S_OK)
				{
					currentServerFolder = pathToFolder;
				}

				CoTaskMemFree(pathToFolder);
			}));
	}

	void MainWindow::applyConfiguration()
	{
		json::JSONParser simpleServerConfiguration = ifstream(constants::simpleServerConfiguration);

		const string& pathToLastApp = simpleServerConfiguration.getString(constants::settings::pathToLastAppSetting);
		const string& language = simpleServerConfiguration.getString(constants::settings::language);

		if (filesystem::exists(pathToLastApp))
		{
			currentServerFolder = pathToLastApp;
		}

		localization::TextLocalization::get().changeLanguage(language);
		localization::WTextLocalization::get().changeLanguage(language);
	}

	bool MainWindow::onClose()
	{
		json::JSONParser configuration = ifstream(constants::simpleServerConfiguration);

		if (configuration.getString(constants::settings::pathToLastAppSetting) != currentServerFolder.string())
		{
			json::JSONBuilder updateLastAppSetting(configuration.getParsedData(), CP_UTF8);
			
			updateLastAppSetting[constants::settings::pathToLastAppSetting] = currentServerFolder.string();

			ofstream(constants::simpleServerConfiguration) << updateLastAppSetting;
		}

		return true;
	}

	void MainWindow::changeServerState()
	{
		serverState ?
			this->stopServer() :
			this->startServer();
	}

	void MainWindow::startServer()
	{
		localization::WTextLocalization& localization = localization::WTextLocalization::get();

		try
		{
			if (!server || (server && server->getConfigurationJSONFile() != filesystem::path(currentServerFolder) / constants::webFrameworkConfiguration))
			{
				server = make_unique<framework::WebFramework>(filesystem::path(currentServerFolder) / constants::webFrameworkConfiguration);
			}
		}
		catch (const exception& e)
		{
			simple_server::utility::showError(e);

			return;
		}

		server->startServer();

		serverState = true;

		serverButton->setText(localization[constants::localization_keys::stopApplicationKey]);

		serverButton->setBackgroundColor(0, 255, 0);
	}

	void MainWindow::stopServer()
	{
		localization::WTextLocalization& localization = localization::WTextLocalization::get();

		serverButton->setText(localization[constants::localization_keys::stoppingApplicationKey]);

		serverButton->setBackgroundColor(128, 128, 128);

		serverButton->disable();

		thread([this, &localization]()
			{
				server->stopServer();

				serverState = false;

				gui_framework::GUIFramework::runOnUIThread([this, &localization]()
					{
						serverButton->setText(localization[constants::localization_keys::startApplicationKey]);

						serverButton->setBackgroundColor(255, 0, 0);

						serverButton->enable();
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
		currentServerFolder(filesystem::current_path()),
		serverFolder(nullptr),
		serverState(false)
	{
		setExitMode(exitMode::quit);

		this->setOnClose(bind(&MainWindow::onClose, this));

		switch (CoCreateInstance(CLSID_FileOpenDialog, nullptr, tagCLSCTX::CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&serverFolderDialog)))
		{
		case S_OK:
			break;

		case REGDB_E_CLASSNOTREG:
			throw runtime_error("A specified class is not registered in the registration database. Also can indicate that the type of server you requested in the CLSCTX enumeration is not registered or the values for the server types in the registry are corrupt");

		case CLASS_E_NOAGGREGATION:
			throw runtime_error("This class cannot be created as part of an aggregate");

		case E_NOINTERFACE:
			throw runtime_error("The specified class does not implement the requested interface, or the controlling IUnknown does not expose the requested interface");

		case E_POINTER:
			throw runtime_error("serverFolderDialog is nullptr");
		}

		this->applyConfiguration();

		this->createMarkup();

		serverFolderDialog->SetOptions(FOS_PICKFOLDERS | FOS_PATHMUSTEXIST);

		IShellItem* folder;

		SHCreateItemFromParsingName(currentServerFolder.wstring().data(), nullptr, IID_PPV_ARGS(&folder));

		serverFolderDialog->SetFolder(folder);

		folder->Release();
	}
}
