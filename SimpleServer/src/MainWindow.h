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
		std::filesystem::path currentServerFolder;
		IFileDialog* serverFolderDialog;
		IShellItem* serverFolder;
		std::unique_ptr<framework::WebFramework> server;
		bool serverState;
		double updatePeriod;

	private:
		gui_framework::Button* serverButton;

	private:
		void createMarkup();

		void applyConfiguration();

		bool onClose();

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
