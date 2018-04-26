#include "StdAfx.h"
#include "utility.h"

std::string utility::GetExeDirectoryA()
{
	char szExePath[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, szExePath, MAX_PATH);
	std::string strExeDir(szExePath);
	std::string::size_type nPos = strExeDir.rfind("\\");
	if (nPos == std::string::npos)
		nPos = strExeDir.rfind("/");
	ASSERT(nPos != std::string::npos);
	strExeDir = strExeDir.substr(0, nPos);
	return strExeDir;
}

std::wstring utility::GetExeDirectoryW()
{
	wchar_t szExePath[MAX_PATH] = { 0 };
	GetModuleFileNameW(NULL, szExePath, MAX_PATH);
	std::wstring strExeDir(szExePath);
	std::wstring::size_type nPos = strExeDir.rfind(L"\\");
	if (nPos == std::wstring::npos)
		nPos = strExeDir.rfind(L"/");
	ASSERT(nPos != std::wstring::npos);
	strExeDir = strExeDir.substr(0, nPos);
	return strExeDir;
}

void utility::Ansi2Unicode(const std::string& strAnsi, std::wstring& strUnicode)
{
	if (strAnsi.empty())
		strUnicode.clear();

	const int nSize = strAnsi.size() + 1;
	ASSERT(nSize > 0);
	wchar_t * pszUnicode = new wchar_t[nSize];
	const int nLength = MultiByteToWideChar(CP_ACP, 0, strAnsi.c_str(), strAnsi.size(), pszUnicode, nSize);
	pszUnicode[nLength] = L'\0';

	strUnicode = pszUnicode;
	delete[]pszUnicode;
}

std::string utility::WideCharToMultiChar(const std::wstring& str)
{
	std::string return_value;

	//获取缓冲区的大小，并申请空间，缓冲区大小是按字节计算的

	int len = WideCharToMultiByte(CP_ACP, 0, str.c_str(), str.size(), NULL, 0, NULL, NULL);

	char *buffer = new char[len + 1];

	WideCharToMultiByte(CP_ACP, 0, str.c_str(), str.size(), buffer, len, NULL, NULL);

	buffer[len] = '\0';

	//删除缓冲区并返回值

	return_value.append(buffer);

	delete[]buffer;

	return return_value;
}