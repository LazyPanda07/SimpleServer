#include "MainWindow.h"

#include <fstream>
#include <psapi.h>

#include "MenuItems/MenuItem.h"
#include "Utility.h"
#include "SimpleServerConstants.h"
#include "MultiLocalizationManager.h"
#include "resource.h"
#include "GUIFramework.h"

using namespace std;

CREATE_DEFAULT_WINDOW_FUNCTION(MainWindow)

namespace simple_server
{
	void MainWindow::createMarkup()
	{
		using gui_framework::utility::ComponentSettings;

		localization::WTextLocalization& localization = localization::WTextLocalization::get();
		unique_ptr<gui_framework::Menu>& menu = this->createMainMenu(L"Menu");
		HANDLE currentProcess = GetCurrentProcess();

		menu->addMenuItem
		(
			make_unique<gui_framework::MenuItem>(localization[constants::localization_keys::chooseApplicationFolderKey], bind(&MainWindow::chooseApplicationFolder, this))
		);

		startStopApplicationButton = new gui_framework::Button(L"Start", localization[constants::localization_keys::startApplicationKey], ComponentSettings(10, 10, 200, 30), this, bind(&MainWindow::changeServerState, this));

		startStopApplicationButton->setBackgroundColor(255, 0, 0);

		gui_framework::StaticControl* ramUsage = new gui_framework::StaticControl(L"RAMUsage", L"", ComponentSettings(300, 10, 200, 20), this);
		gui_framework::StaticControl* cpuUsage = new gui_framework::StaticControl(L"CPUUsage", L"", ComponentSettings(300, 30, 200, 20), this);

		ramUsage->setBackgroundColor(240, 240, 240);
		cpuUsage->setBackgroundColor(240, 240, 240);

		this->updateRAMUsage(ramUsage, currentProcess);

		this->updateCPUUsage(cpuUsage, currentProcess);

		addTrayMenuItem(L"Exit", [this]() { this->destroyComponent(); });
	}

	void MainWindow::applyConfiguration()
	{
		json::JSONParser simpleServerConfiguration = ifstream(constants::simpleServerConfiguration);

		const string& pathToLastApp = simpleServerConfiguration.getString(constants::settings::pathToLastAppSetting);
		const string& language = simpleServerConfiguration.getString(constants::settings::languageSetting);

		if (filesystem::exists(pathToLastApp))
		{
			filesystem::path previousApplicatorFolder = currentApplicationFolder;

			currentApplicationFolder = pathToLastApp;

			this->onCurrentApplicationFolderChange(previousApplicatorFolder);
		}

		updatePeriod = simpleServerConfiguration.getDouble(constants::settings::updateStatsPeriodInSecondsSetting);

		localization::TextLocalization::get().changeLanguage(language);
		localization::WTextLocalization::get().changeLanguage(language);
	}

	void MainWindow::registerHotkeys()
	{
		using gui_framework::hotkeys::keys;

		gui_framework::GUIFramework& instance = gui_framework::GUIFramework::get();

		instance.registerHotkey(keys::enter, bind(&MainWindow::changeServerState, this));

		instance.registerHotkey(keys::escape, bind(&MainWindow::destroyComponent, this));

		instance.registerHotkey(keys::O, bind(&MainWindow::chooseApplicationFolder, this), { gui_framework::hotkeys::additionalKeys::control });
	}

	void MainWindow::onDestroyEvent()
	{
		json::JSONParser configuration = ifstream(constants::simpleServerConfiguration);

		if (configuration.getString(constants::settings::pathToLastAppSetting) != currentApplicationFolder.string())
		{
			json::JSONBuilder updateLastAppSetting(configuration.getParsedData(), CP_UTF8);
			
			updateLastAppSetting[constants::settings::pathToLastAppSetting] = currentApplicationFolder.string();

			ofstream(constants::simpleServerConfiguration) << updateLastAppSetting;
		}

		this->stopServer();
	}

	void MainWindow::chooseApplicationFolder()
	{
		if (server && server->getServerState())
		{
			utility::showError(runtime_error(localization::TextLocalization::get()[constants::localization_keys::changeFolderErrorKey]));

			return;
		}

		if (applicationFolderDialog->Show(nullptr) != S_OK)
		{
			return;
		}

		if (applicationFolderDialog->GetResult(&applicationFolder) != S_OK)
		{
			applicationFolder = nullptr;

			return;
		}

		PWSTR pathToFolder;

		if (applicationFolder->GetDisplayName(SIGDN::SIGDN_DESKTOPABSOLUTEEDITING, &pathToFolder) == S_OK)
		{
			filesystem::path previousApplicatorFolder = currentApplicationFolder;

			currentApplicationFolder = pathToFolder;

			this->onCurrentApplicationFolderChange(previousApplicatorFolder);

			CoTaskMemFree(pathToFolder);
		}
	}

	void MainWindow::onCurrentApplicationFolderChange(const filesystem::path& previousApplicationFolder)
	{
		utility::throwExceptionIfFileDoesNotExist(currentApplicationFolder / constants::applicationLocalizationModules);

		json::JSONParser applicationLocalizationModules;
		localization::MultiLocalizationManager& manager = localization::MultiLocalizationManager::getManager();

		if (filesystem::exists(previousApplicationFolder / constants::applicationLocalizationModules))
		{
			applicationLocalizationModules.setJSONData(ifstream(previousApplicationFolder / constants::applicationLocalizationModules));

			for (const string& moduleToRemove : json::utility::JSONArrayWrapper(applicationLocalizationModules.getArray(constants::settings::modulesSetting)).getAsStringArray())
			{
				manager.removeModule(moduleToRemove);
			}
		}

		applicationLocalizationModules.setJSONData(ifstream(currentApplicationFolder / constants::applicationLocalizationModules));

		for (const string& moduleToAdd : json::utility::JSONArrayWrapper(applicationLocalizationModules.getArray(constants::settings::modulesSetting)).getAsStringArray())
		{
			manager.addModule(filesystem::path(moduleToAdd).filename().string(), currentApplicationFolder / (moduleToAdd + ".dll"));
		}

		SetWindowTextW(handle, format(L"{} - {}", windowName, currentApplicationFolder.wstring()).data());
	}

	void MainWindow::updateRAMUsage(gui_framework::StaticControl* staticControl, HANDLE currentProcess)
	{
		localization::WTextLocalization& localization = localization::WTextLocalization::get();

		thread([this, staticControl, currentProcess, &localization]()
			{
				while (true)
				{
					PROCESS_MEMORY_COUNTERS_EX memory = {};

					GetProcessMemoryInfo(currentProcess, reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&memory), sizeof(memory));

					staticControl->setText
					(
						format
						(
							L"{:.1f} {} {}",
							static_cast<double>(memory.PrivateUsage) / 1024 / 1024,
							localization[constants::localization_keys::MiBKey],
							localization[constants::localization_keys::memoryUsageDescriptionKey]
						)
					);

					this_thread::sleep_for(chrono::duration<double>(updatePeriod));
				}
			}).detach();
	}

	void MainWindow::updateCPUUsage(gui_framework::StaticControl* staticControl, HANDLE currentProcess)
	{
		static ULARGE_INTEGER lastCPU;
		static ULARGE_INTEGER lastSysCPU;
		static ULARGE_INTEGER lastUserCPU;
		static DWORD numProcessors;

		FILETIME ftime;
		FILETIME fsys;
		FILETIME fuser;

		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);
		numProcessors = sysInfo.dwNumberOfProcessors;

		GetSystemTimeAsFileTime(&ftime);
		memcpy(&lastCPU, &ftime, sizeof(ftime));

		GetProcessTimes(currentProcess, &ftime, &ftime, &fsys, &fuser);
		memcpy(&lastSysCPU, &fsys, sizeof(fsys));
		memcpy(&lastUserCPU, &fuser, sizeof(fuser));

		thread([this, staticControl, currentProcess]()
			{
				while (true)
				{
					ULARGE_INTEGER now;
					ULARGE_INTEGER sys;
					ULARGE_INTEGER user;
					FILETIME ftime;
					FILETIME fsys;
					FILETIME fuser;
					double percent;

					GetSystemTimeAsFileTime(&ftime);
					memcpy(&now, &ftime, sizeof(ftime));

					GetProcessTimes(currentProcess, &ftime, &ftime, &fsys, &fuser);
					memcpy(&sys, &fsys, sizeof(fsys));
					memcpy(&user, &fuser, sizeof(fuser));

					percent = static_cast<double>((sys.QuadPart - lastSysCPU.QuadPart) + (user.QuadPart - lastUserCPU.QuadPart)) / (now.QuadPart - lastCPU.QuadPart) / numProcessors;

					lastCPU = now;
					lastUserCPU = user;
					lastSysCPU = sys;

					staticControl->setText(format(L"{:.1f}% CPU", percent >= 0.0f ? (percent * 100.0) : 0.0));

					this_thread::sleep_for(chrono::duration<double>(updatePeriod));
				}
			}).detach();
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
			if (!server || (server && server->getConfigurationJSONFile() != filesystem::path(currentApplicationFolder) / constants::webFrameworkConfiguration))
			{
				SetDllDirectoryW(currentApplicationFolder.wstring().data());

				server = make_unique<framework::WebFramework>(filesystem::path(currentApplicationFolder) / constants::webFrameworkConfiguration);
			}
		}
		catch (const exception& e)
		{
			simple_server::utility::showError(e);

			return;
		}

		server->startServer();

		serverState = true;

		startStopApplicationButton->setText(localization[constants::localization_keys::stopApplicationKey]);

		startStopApplicationButton->setBackgroundColor(0, 255, 0);
	}

	void MainWindow::stopServer()
	{
		if (!serverState)
		{
			return;
		}

		localization::WTextLocalization& localization = localization::WTextLocalization::get();

		startStopApplicationButton->setText(localization[constants::localization_keys::stoppingApplicationKey]);

		startStopApplicationButton->setBackgroundColor(128, 128, 128);

		startStopApplicationButton->disable();

		thread([this, &localization]()
			{
				server->stopServer();

				serverState = false;

				gui_framework::GUIFramework::runOnUIThread([this, &localization]()
					{
						startStopApplicationButton->setText(localization[constants::localization_keys::startApplicationKey]);

						startStopApplicationButton->setBackgroundColor(255, 0, 0);

						startStopApplicationButton->enable();
					});
			}).detach();
	}

	MainWindow::MainWindow() :
		BaseMainWindow
		(
			L"SimpleServer",
			L"Simple Server",
			gui_framework::utility::ComponentSettings
			(
				gui_framework::utility::getCenterX(800),
				gui_framework::utility::getCenterY(600),
				800,
				600
			),
			"MainWindow",
			APP_ICON,
			true,
			false,
			false,
			"",
			APP_ICON,
			APP_ICON
		),
		applicationFolder(nullptr),
		serverState(false)
	{
		disableResize();

		setBackgroundColor(240, 240, 240);

		switch (CoCreateInstance(CLSID_FileOpenDialog, nullptr, tagCLSCTX::CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&applicationFolderDialog)))
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
			throw runtime_error("applicationFolderDialog is nullptr");
		}

		this->applyConfiguration();

		this->registerHotkeys();

		applicationFolderDialog->SetOptions(FOS_PICKFOLDERS | FOS_PATHMUSTEXIST);

		IShellItem* folder;

		SHCreateItemFromParsingName(filesystem::current_path().wstring().data(), nullptr, IID_PPV_ARGS(&folder));

		applicationFolderDialog->SetFolder(folder);

		folder->Release();
	}

	void MainWindow::applyCommandLine(PWSTR commandLineArguments)
	{
		int argc;
		PWSTR* argv = CommandLineToArgvW(commandLineArguments, &argc);

		for (int i = 0; i < argc; i++)
		{
			wstring_view command = argv[i];

			if (command == L"-startServer")
			{
				this->startServer();
			}
			else if (command == L"-tray")
			{
				onClose();
			}
		}
	}
}
