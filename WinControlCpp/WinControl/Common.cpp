#include "stdafx.h"
#include "Common.h"
#include "LogFile.h"

//降序排序函数
bool DescSort(const ORDERS_TEN_RESULTS& V1, const ORDERS_TEN_RESULTS& V2)
{
	return V1.uiAllTotalBonus > V2.uiAllTotalBonus;
}
//升序排序函数
bool AscSort(const ORDERS_TEN_RESULTS& V1, const ORDERS_TEN_RESULTS& V2)
{
	return V1.uiAllTotalBonus < V2.uiAllTotalBonus;
}

void CreatePath(char szLogPath[MAX_PATH])
{
	if (0 == szLogPath[0])
	{
		printf("CreatePath Error!\n");
		return;
	}
	char szTempLogPath[MAX_PATH];
	memset(szTempLogPath, 0, sizeof(szTempLogPath));
	strncpy_s(szTempLogPath, szLogPath, sizeof(szTempLogPath) - 1);
	int iTempLen = strlen(szTempLogPath);

	if ('/' == szTempLogPath[iTempLen - 1])
	{
		szTempLogPath[iTempLen - 1] = 0;
	}
	char *pTemp = szTempLogPath;
	char *pTemp1;
	char chTemp;
	while (1)
	{
		pTemp1 = strchr(pTemp, '/');
		if (0 == pTemp1)
		{
			_mkdir(szTempLogPath);
			break;
		}
		if (0 == *(pTemp1 + 1))
		{
			break;
		}

		chTemp = *(pTemp1 + 1);
		*(pTemp1 + 1) = 0;
		if (0 != strcmp("../", szTempLogPath) && 0 != strcmp("./", szTempLogPath))
		{
			_mkdir(szTempLogPath);
		}
		*(pTemp1 + 1) = chTemp;
		pTemp = pTemp1 + 1;
	}
}

bool GetCurrentWorkDir(char* pPath, UINT32 iSize)
{
	bool ret = true;
	if (getcwd(pPath, iSize) == NULL)
	{
		ret = false;
	}
		
	return ret;
}
void QueryCurrentTime(CurrentTime* pCurrentTime)
{
	struct tm tmpTime = {0};
	struct timeval tmNow;

#ifndef WIN32
	gettimeofday(&tmNow, NULL);
#else
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);
	tmNow.tv_sec = (long)time(NULL);
	tmNow.tv_usec = sysTime.wMilliseconds * 1000;
#endif
	time_t tNow = tmNow.tv_sec;
	pCurrentTime->ulMSecond = tmNow.tv_usec / 1000;

	localtime_s(&tmpTime, &tNow);
	pCurrentTime->ulYear = tmpTime.tm_year + 1900;
	pCurrentTime->ulMonth = tmpTime.tm_mon + 1;
	pCurrentTime->ulDay = tmpTime.tm_mday;
	pCurrentTime->ulHour = tmpTime.tm_hour;
	pCurrentTime->ulMinute = tmpTime.tm_min;
	pCurrentTime->ulSecond = tmpTime.tm_sec;
}

std::string string_To_UTF8(const std::string & str)
{
	int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

	wchar_t * pwBuf = new wchar_t[nwLen + 1];//一定要加1，不然会出现尾巴 
	ZeroMemory(pwBuf, nwLen * 2 + 2);

	::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);

	int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

	char * pBuf = new char[nLen + 1];
	ZeroMemory(pBuf, nLen + 1);

	::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

	std::string retStr(pBuf);

	delete[]pwBuf;
	delete[]pBuf;

	pwBuf = NULL;
	pBuf = NULL;

	return retStr;
}

std::string UTF8_To_string(const std::string & str)
{
	int nwLen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);

	wchar_t * pwBuf = new wchar_t[nwLen + 1];//一定要加1，不然会出现尾巴 
	memset(pwBuf, 0, nwLen * 2 + 2);

	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), pwBuf, nwLen);

	int nLen = WideCharToMultiByte(CP_ACP, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

	char * pBuf = new char[nLen + 1];
	memset(pBuf, 0, nLen + 1);

	WideCharToMultiByte(CP_ACP, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

	std::string retStr = pBuf;

	delete[]pBuf;
	delete[]pwBuf;

	pBuf = NULL;
	pwBuf = NULL;

	return retStr;
}

