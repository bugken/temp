#include "stdafx.h"
#include <winsock2.h>
#include <windows.h>
#include "LogFile.h"
#include "win_linux.h"

#ifndef WIN32
#include <unistd.h>
#else
#include <io.h>
#include <direct.h>
#endif

#ifndef  WIN32
#ifndef MAX_PATH
#define MAX_PATH 260
#endif
#include <sys/time.h>
#endif

#ifdef WIN32
#define snprintf _snprintf
#endif

CLogFile::CLogFile()
{
	memset(m_szLogPath, 0, sizeof(m_szLogPath));
	memset(m_szBakLogPath, 0, sizeof(m_szBakLogPath));
	memset(m_szLogName, 0, sizeof(m_szLogName));
	memset(m_szErrLogName, 0, sizeof(m_szErrLogName));
	memset(m_szInfoLogName, 0, sizeof(m_szInfoLogName));
	memset(&m_currentTime, 0, sizeof(m_currentTime));
}

CLogFile::~CLogFile()
{
}

void CLogFile::SetLogPath(const char *pLogPath)
{
	if (!pLogPath)
	{
		return;
	}

	memset(m_szLogPath, 0, sizeof(m_szLogPath));
	strncpy(m_szLogPath, pLogPath, sizeof(m_szLogPath) - 2);
	int nLen = (int)strlen(m_szLogPath);
	for (int i = 0; i < nLen; i++)
	{
		if (m_szLogPath[i] == '\\')
		{
			m_szLogPath[i] = '/';
		}
	}
	if (m_szLogPath[nLen - 1] != '/')
	{
		m_szLogPath[nLen] = '/';
	}

	char szPath[MAX_PATH] = { 0 };
	snprintf(szPath, sizeof(szPath) - 1, "%sbaklogs", m_szLogPath);
	SetBakLogPath(szPath);
}

void CLogFile::SetBakLogPath(const char *pBakLogPath)
{
	if (!pBakLogPath)
	{
		return;
	}
	memset(m_szBakLogPath, 0, sizeof(m_szBakLogPath));
	strncpy(m_szBakLogPath, pBakLogPath, sizeof(m_szBakLogPath) - 2);
	int nLen = (int)strlen(m_szBakLogPath);
	for (int i = 0; i < nLen; i++)
	{
		if (m_szBakLogPath[i] == '\\')
		{
			m_szBakLogPath[i] = '/';
		}
	}
	if (m_szBakLogPath[nLen - 1] != '/')
	{
		m_szBakLogPath[nLen] = '/';
	}
}

void CLogFile::SetLogName(const char* pLogName)
{
	if (!pLogName)
		return;

	memset(m_szErrLogName, 0, sizeof(m_szErrLogName));
	memset(m_szInfoLogName, 0, sizeof(m_szInfoLogName));
	strncpy(m_szLogName, pLogName, sizeof(m_szLogName) - 1);
	snprintf(m_szErrLogName, sizeof(m_szErrLogName) - 1, "%s_err.log", m_szLogName);
	snprintf(m_szInfoLogName, sizeof(m_szInfoLogName) - 1, "%s_info.log", m_szLogName);
}

void CLogFile::SetCurrentTime()
{
	struct tm *tmpTime = NULL;
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
	m_currentTime.ulMSecond = tmNow.tv_usec / 1000;
	tmpTime = localtime(&tNow);

	m_currentTime.ulYear = tmpTime->tm_year + 1900;
	m_currentTime.ulMonth = tmpTime->tm_mon + 1;
	m_currentTime.ulDay = tmpTime->tm_mday;
	m_currentTime.ulHour = tmpTime->tm_hour;
	m_currentTime.ulMinute = tmpTime->tm_min;
	m_currentTime.ulSecond = tmpTime->tm_sec;
}

void CLogFile::BackupFile(const char* pSrcFile, const char* pDstFile)
{
	if (access(pSrcFile, 0) != -1)
	{
		char szBakLogPath[MAX_PATH] = { 0 };
		strncpy(szBakLogPath, pDstFile, MAX_PATH - 1);
		char* pEnd = strrchr(szBakLogPath, '/');
		if (pEnd)
		{
			*pEnd = 0;
		}
		if (access(szBakLogPath, 0) != 0)
		{
			CreatePath(szBakLogPath);
		}

		unlink(pDstFile);
		rename(pSrcFile, pDstFile);
		unlink(pSrcFile);
	}
}


void CLogFile::ErrorLog(const char* msg, ...)
{
	SetCurrentTime();
	va_list args;

	va_start(args, msg);
	WriteToLogFile(m_szErrLogName, msg, args);
	va_end(args);
}

void CLogFile::InfoLog(const char* msg, ...)
{
	if (0 == m_szInfoLogName[0])
	{
		return;
	}
	SetCurrentTime();

	va_list args;
	va_start(args, msg);
	WriteToLogFile(m_szInfoLogName, msg, args);
	va_end(args);
}

void CLogFile::WriteToLogFile(const char* pFileName, const char* msg, va_list& args, char *pAddStr)
{
	LockGuard lockGuard(m_Mutex);
	char szLogFile[MAX_PATH];
	sprintf(szLogFile, "%s%s", m_szLogPath, pFileName);

	FILE *pFile = fopen(szLogFile, "a+");
	if (NULL == pFile)
	{
		char szTempLogFile[MAX_PATH];
		strncpy(szTempLogFile, szLogFile, sizeof(szTempLogFile) - 1);
		char *pTemp = strrchr(szTempLogFile, '/');
		if (pTemp == NULL)
		{
			return;
		}
		*pTemp = 0;
		if (access(szTempLogFile, 0) != 0)
		{
			CreatePath(szTempLogFile);
		}
		pFile = fopen(szLogFile, "a+");
	}
	if (pFile)
	{
		WriteLog(pFile, msg, args, pAddStr);
		fclose(pFile);
	}
}

void CLogFile::WriteLog(FILE* pFile, const char *msg, va_list& args, char* pAddStr)
{
	fprintf(pFile, "[%.4u-%.2u-%.2u,%.2u:%.2u:%.2u:%.3u] ",
		m_currentTime.ulYear, m_currentTime.ulMonth, m_currentTime.ulDay,
		m_currentTime.ulHour, m_currentTime.ulMinute,
		m_currentTime.ulSecond, m_currentTime.ulMSecond);

	if (pAddStr)
	{
		fprintf(pFile, "[%s]", pAddStr);
	}

	vfprintf(pFile, msg, args);
	//fprintf(pFile,"\n");
}

//����Log���ȼ�д���ļ�,Ĭ��д��m_szLogName
void CLogFile::WriteLogFile(int nPriority, const char* msg, ...)
{
	char* pAddStr = NULL;
	switch (nPriority)
	{
	case 1:
		pAddStr = (char*)"[info]";
		break;
	default:
		return;
	}

	SetCurrentTime();
	LockGuard lockGuard(m_Mutex);
	char szLogFile[MAX_PATH];
	snprintf(szLogFile, sizeof(szLogFile) - 1, "%s%s", m_szLogPath, m_szLogName);
	szLogFile[sizeof(szLogFile) - 1] = 0;
	FILE *pFile = fopen(szLogFile, "a+");
	if (NULL == pFile)
	{
		char szTempLogFile[MAX_PATH];
		strncpy(szTempLogFile, szLogFile, sizeof(szTempLogFile) - 1);
		char *pTemp = strrchr(szTempLogFile, '/');
		if (pTemp == NULL)
		{
			return;
		}
		*pTemp = 0;
		if (access(szTempLogFile, 0) != 0)
		{
			CreatePath(szTempLogFile);
		}
		pFile = fopen(szLogFile, "a+");
	}
	if (pFile)
	{
		va_list   args;
		va_start(args, msg);
		WriteLog(pFile, msg, args, pAddStr);
		va_end(args);
		fclose(pFile);
	}
}

void CLogFile::LogToFile(const char* pszLogFile, const char* msg, ...)
{
	if (msg == NULL || pszLogFile == NULL)
	{
		return;
	}

	SetCurrentTime();
	va_list args;
	va_start(args, msg);
	WriteToLogFile(pszLogFile, msg, args);
	va_end(args);
}

void CLogFile::LogToFileByDay(const char* pszLogFile, const char* msg, ...)
{
	if (msg == NULL || pszLogFile == NULL)
	{
		return;
	}

	char m_szLogToFileName[MAX_PATH];
	memset(m_szLogToFileName, 0, sizeof(m_szLogToFileName));
	SetCurrentTime();
	int nRet = snprintf(m_szLogToFileName, sizeof(m_szLogToFileName) - 1, "%04u-%02u-%02u/%s",
			m_currentTime.ulYear, m_currentTime.ulMonth, m_currentTime.ulDay, pszLogFile);
	if (nRet <= 0)
	{
		return;
	}
	m_szLogToFileName[nRet] = '\0';

	va_list args;
	va_start(args, msg);
	WriteToLogFile(m_szLogToFileName, msg, args);
	va_end(args);
}

void CLogFile::InitLogCfg(const char* pszCfgFile, char* section, char* preStr, char* path)
{
	ReadCfg();
}

void CLogFile::ReadCfg()
{

}

void CLogFile::ReloadCfg()
{
	LockGuard lockGuard(m_Mutex);
	ReadCfg();
}
