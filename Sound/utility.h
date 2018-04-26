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
char(&arrayCountHelper(T(&array)[N]))[N];  //�ú���ģ�嶼����Ҫʵ�֣�ֻ�趨�弴��

										   //�ڱ����ڼ�������ĳ���
#define arrayCount(array) (sizeof( arrayCountHelper(array) ) )

