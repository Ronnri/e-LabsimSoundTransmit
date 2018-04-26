#pragma once

#include <string>
namespace utility
{
	std::string GetExeDirectoryA();
	std::wstring GetExeDirectoryW();
	void Ansi2Unicode(const std::string& strAnsi, std::wstring& strUnicode);
	std::string WideCharToMultiChar(const std::wstring& str);
}


template< typename T, int N >
char(&arrayCountHelper(T(&array)[N]))[N];  //该函数模板都不需要实现，只需定义即可

										   //在编译期计算数组的长度
#define arrayCount(array) (sizeof( arrayCountHelper(array) ) )

