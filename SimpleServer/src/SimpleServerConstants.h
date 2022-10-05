#pragma once

#include <string>

namespace simple_server
{
	namespace constants
	{
		inline constexpr std::string_view webFrameworkConfiguration = "app_configuration.json";

		inline const std::string simpleServerConfiguration = "simple_server_configuration.json";

		namespace settings
		{
			inline const std::string language = "language";
			inline const std::string pathToLastAppSetting = "pathToLastApp";
		}

		namespace localization_keys
		{
			inline const std::string startApplicationKey = "startApplication";
			inline const std::string stopApplicationKey = "stopApplication";
			inline const std::string stoppingApplicationKey = "stoppingApplication";
			inline const std::string chooseApplicationFolderKey = "chooseApplicationFolder";
			inline const std::string changeFolderErrorKey = "changeFolderError";
			inline const std::string errorTitleKey = "errorTitle";
		}
	}
}
