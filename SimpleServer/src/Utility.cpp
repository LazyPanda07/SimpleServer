#include "Utility.h"

#include "CompositesHeader.h"
#include "SimpleServerConstants.h"

using namespace std;

namespace simple_server
{
	namespace utility
	{
		void showError(const exception& error)
		{
			using gui_framework::BaseDialogBox;

			BaseDialogBox::createMessageBox
			(
				gui_framework::utility::to_wstring(error.what(), CP_UTF8),
				localization::WTextLocalization::get()[constants::localization_keys::errorTitleKey],
				BaseDialogBox::messageBoxType::ok
			);
		}
	}
}
