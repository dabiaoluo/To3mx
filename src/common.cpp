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
			printf(l_cLog);
			printf("\n");
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