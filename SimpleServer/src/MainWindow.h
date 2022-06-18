#pragma once

#include "CompositesHeader.h"

namespace simple_server
{
	class MainWindow : public gui_framework::SeparateWindow
	{
	private:
		static int getCenterX(int width);

		static int getCenterY(int height);

	public:
		MainWindow();

		~MainWindow() = default;
	};
}
