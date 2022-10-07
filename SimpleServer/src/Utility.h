#pragma once

#include <exception>
#include <filesystem>

namespace simple_server
{
	namespace utility
	{
		void showError(const std::exception& error);

		void throwExceptionIfFileDoesNotExist(const std::filesystem::path& pathToFile);
	}
}
