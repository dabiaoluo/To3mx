#include "Common.h"
#include <experimental/filesystem>
#include <algorithm>
#include <iostream>
#include <stdarg.h>

namespace seed
{
	namespace log
	{
		bool DumpLog(const int&		i_nType,	// log type
			const char*		i_cFormat,
			...)		
		{
			if (i_nType == Debug)
			{
#ifndef _DEBUG
				return true;
#endif
			}
			const int MAX_LOG_SIZE = 512;
			char l_cLog[MAX_LOG_SIZE];
			va_list args;
			va_start(args, i_cFormat);
			_vsnprintf_s(l_cLog, _TRUNCATE, i_cFormat, args);
			va_end(args);
			switch (i_nType)
			{
			case Debug:
				printf("Debug: %s\n", l_cLog);
				break;
			case Warning:
				printf("Warning: %s\n", l_cLog);
				break;
			case Critical:
				printf("Critical: %s\n", l_cLog);
				break;
			case Fatal:
				printf("Fatal: %s\n", l_cLog);
				break;
			case Info:
				printf("Info: %s\n", l_cLog);
				break;
			default:
				break;
			}
			return true;
		}
	}

	namespace progress
	{
		void UpdateProgress(int value)
		{
			std::cout << "Progress: " << value << std::endl;
		}
	}

	namespace utils
	{
		bool CheckOrCreateFolder(const std::string& i_strDir)
		{
			if (std::experimental::filesystem::exists(i_strDir))
			{
				return true;
			}
			if (std::experimental::filesystem::create_directories(i_strDir) == false)
			{
				seed::log::DumpLog(seed::log::Critical, "Create folder %s failed!", i_strDir.c_str());
				return false;
			}
			else
			{
				return true;
			}
		}

		bool FileExists(const std::string& i_strPath)
		{
			return std::experimental::filesystem::exists(i_strPath);
		}
	}
}