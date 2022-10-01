#include "Utility.h"

#include "CompositesHeader.h"

using namespace std;

namespace simple_server
{
	namespace utility
	{
		void showError(const exception& error)
		{
			using gui_framework::BaseDialogBox;

			BaseDialogBox::createMessageBox(gui_framework::utility::to_wstring(error.what(), CP_ACP), L"Error", BaseDialogBox::messageBoxType::ok);
		}
	}
}
