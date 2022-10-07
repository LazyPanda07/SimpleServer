#pragma once

#include "WebFramework.h"

#include <shobjidl_core.h>

#include "CompositesHeader.h"
#include "ComponentsHeader.h"

namespace simple_server
{
	class MainWindow : public gui_framework::SeparateWindow
	{
	private:
		static int getCenterX(int width);

		static int getCenterY(int height);

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

		bool onClose();

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
