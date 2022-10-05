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
		gui_framework::Button* serverButton;
		std::unique_ptr<framework::WebFramework> server;
		bool serverState;

	private:
		void createMarkup();

		void applyConfiguration();

		bool onClose();

	private:
		void changeServerState();

		void startServer();

		void stopServer();

	public:
		MainWindow();

		~MainWindow() = default;
	};
}
