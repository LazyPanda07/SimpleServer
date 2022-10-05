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
			inline const std::string pathToLastAppSetting = "pathToLastApp";
		}
	}
}
