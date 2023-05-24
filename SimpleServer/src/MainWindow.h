#pragma once

#include "WebFramework.h"

#include <shobjidl_core.h>

#include "BaseComposites/StandardComposites/BaseMainWindow.h"
#include "ComponentsHeader.h"

namespace simple_server
{
	class MainWindow : public gui_framework::BaseMainWindow
	{
	private:
		std::filesystem::path currentApplicationFolder;
		IFileDialog* applicationFolderDialog;
		IShellItem* applicationFolder;
		std::unique_ptr<framework::WebFramework> server;
		bool serverState;
		double updatePeriod;

	private:
		gui_framework::Button* startStopApplicationButton;

	private:
		void createMarkup();

		void applyConfiguration();

		void registerHotkeys();

		void onDestroyEvent();

		void chooseApplicationFolder();

		void onCurrentApplicationFolderChange(const std::filesystem::path& previousApplicationFolder);

		void updateRAMUsage(gui_framework::StaticControl* staticControl, HANDLE currentProcess);

		void updateCPUUsage(gui_framework::StaticControl* staticControl, HANDLE currentProcess);

	private:
		void changeServerState();

		void startServer();

		void stopServer();

	public:
		MainWindow();

		~MainWindow() = default;
	};
}
