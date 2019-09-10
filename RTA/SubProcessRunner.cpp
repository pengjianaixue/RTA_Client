// SubProcessRunner.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "subProcessRunner.h"
subProcessRunner::subProcessRunner()
{

}

bool subProcessRunner::startRun(const string &subprogrammcmd)
{	
	if (!m_bisFinished)
	{
		return false;
	}
	if(!initMemberVar())
		return false;
	createStartUpInfo(&m_siStartInfo);
	
#ifdef UNICODE
	WCHAR szCommmandLine[512] = {0};
	::MultiByteToWideChar(CP_ACP, 0, subprogrammcmd.c_str(), subprogrammcmd.length() + 1, szCommmandLine,
		sizeof(szCommmandLine) / sizeof(szCommmandLine[0]));
#else
	CHAR wszClassName[512] = {0};
	::memcpy_s(wszClassName, subprogrammcmd.length(), subprogrammcmd.c_str(), m_strSubProgrammcmd.length());
#endif // UNICODE
	DWORD dwCreationFlags = CREATE_NO_WINDOW;//(GetConsoleWindow() ? 0 : CREATE_NO_WINDOW);
	dwCreationFlags |= CREATE_UNICODE_ENVIRONMENT;
	dwCreationFlags |= CREATE_SUSPENDED;
	if (!::CreateProcess(NULL,
		szCommmandLine, &m_saAttr, &m_saAttr, TRUE,
		dwCreationFlags, NULL, NULL,
		&m_siStartInfo, &m_piProcInfo))
	{
		CloseHandle(m_hStdInRead);
		CloseHandle(m_hStdOutWrite);
		m_hStdInRead = NULL;
		m_hStdOutWrite = NULL;
#ifdef _DEBUG
		if (m_readCallbackfun)
		{
			m_readCallbackfun(string("DuplicateHandle Pipe fail ,the error code :") + std::to_string(GetLastError()) + "\r\n", this);
		}
		printf("CreateProcess fail ,error code: %d\r\n", GetLastError());
#endif // _DEBUG
		return false;
	}
	
	ResumeThread(m_piProcInfo.hThread);
	m_bisFinished = false;
	m_readSubProcessOutputThread = std::make_shared<std::thread>(&subProcessRunner::readSubProcessOutput,this);
	m_processStatesNotifyThread  = std::make_shared<std::thread>(&subProcessRunner::_isFinished, this);
	return true;
}

bool subProcessRunner::isFinished() const
{	
	return m_bisFinished;
}
int subProcessRunner::waitForFinish()
{
	::WaitForSingleObject(m_piProcInfo.hProcess, INFINITE);
	DWORD dwExitCode = 0;
	m_bisFinished = true;
	if (m_processStatesNotifyThread && m_processStatesNotifyThread->joinable())
	{
		m_processStatesNotifyThread->join();
	}
	::GetExitCodeProcess(m_piProcInfo.hProcess, &dwExitCode);
	return dwExitCode;
}
bool subProcessRunner::registerReadCallBackFuntion(READSTDOUTCALLBACKFUN callBackfunction)
{
	if (callBackfunction)
	{
		m_readCallbackfun = callBackfunction;
		return true;
	}
	return false;
}
bool subProcessRunner::pause()
{
	if (m_piProcInfo.hThread)
	{
		 int ret = ::SuspendThread(m_piProcInfo.hThread);
		 return !ret;
	}
	return false;
}
bool subProcessRunner::resume()
{
	if (m_piProcInfo.hThread)
	{
		int ret = ::ResumeThread(m_piProcInfo.hThread);
		return !ret;
	}
	return false;
}
bool subProcessRunner::stop()
{
	if (m_piProcInfo.hThread && m_piProcInfo.hProcess)
	{
		TerminateThread(m_piProcInfo.hThread,-1);
		TerminateProcess(m_piProcInfo.hProcess, -1);
		return true;
	}
	return false;
}
string subProcessRunner::wirteCmdtoSubprogramm()
{
	return string();
}

void subProcessRunner::readPipeOverlappedCallbackfun(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
{

}

subProcessRunner::~subProcessRunner()
{
	if (m_processStatesNotifyThread && m_processStatesNotifyThread->joinable())
	{
		m_processStatesNotifyThread->join();
		m_processStatesNotifyThread = nullptr;
	}
}

void subProcessRunner::createSecurityAttributes(PSECURITY_ATTRIBUTES pSa)
{
	
	pSa->bInheritHandle = TRUE;
	pSa->lpSecurityDescriptor = NULL;
	pSa->nLength = sizeof(SECURITY_ATTRIBUTES);
}

bool subProcessRunner::initMemberVar()
{
	createSecurityAttributes(&m_saAttr);
	if (!::CreatePipe(&m_hStdInRead, &m_hStdOutWrite, &m_saAttr, 0))
		return false;
	if (!DuplicateHandle(GetCurrentProcess(), GetStdHandle(STD_OUTPUT_HANDLE), GetCurrentProcess(),
	&m_hStdOutWrite, 0, TRUE, DUPLICATE_SAME_ACCESS))
	{
		if (m_readCallbackfun)
		{
			m_readCallbackfun(string("DuplicateHandle Pipe fail ,the error code :") + std::to_string(GetLastError()) + "\r\n", this);
		}
		//return false;
	}
	if (!DuplicateHandle(GetCurrentProcess(), GetStdHandle(STD_INPUT_HANDLE), GetCurrentProcess(),
	&m_hStdInRead, 0, TRUE, DUPLICATE_SAME_ACCESS))
	{
		if (m_readCallbackfun)
		{
			m_readCallbackfun(string("DuplicateHandle Pipe fail ,the error code :") + std::to_string(GetLastError()) + "\r\n", this);
		}
		//return false;
	}
	return true;
}

void subProcessRunner::createStartUpInfo(LPSTARTUPINFO lpStartUpInfo)
{
	ZeroMemory(lpStartUpInfo,sizeof(STARTUPINFO));
	*lpStartUpInfo = { sizeof(STARTUPINFO), 0, 0, 0,
		(unsigned long)CW_USEDEFAULT, (unsigned long)CW_USEDEFAULT,
		(unsigned long)CW_USEDEFAULT, (unsigned long)CW_USEDEFAULT,
		0, NULL, 0,
		STARTF_USESTDHANDLES,
		0, 0, 0,
		m_hStdInRead, m_hStdOutWrite, m_hStdOutWrite
	};
	//lpStartUpInfo->cb = sizeof(STARTUPINFO);
	//lpStartUpInfo->dwFlags = STARTF_USESTDHANDLES;
	//lpStartUpInfo->hStdInput = m_hStdInRead;
	//lpStartUpInfo->hStdOutput = m_hStdOutWrite;
	//lpStartUpInfo->hStdError = m_hStdOutWrite;//GetStdHandle(STD_ERROR_HANDLE);

}

void subProcessRunner::readSubProcessOutput()
{

	char out_buffer[10000] = {0};
	DWORD  dwRead = {0};
	while (!m_bisFinished)
	{
		DWORD bytes = 0;
		/*while (!bytes)
		{
			PeekNamedPipe(m_hStdOutWrite, nullptr, 0, nullptr, &bytes, nullptr);
			Sleep(1);
			if (m_readCallbackfun)
			{
				m_readCallbackfun(string("Read Pipe fail ,the error code :") + std::to_string(GetLastError()) + "\r\n", this);
			}
		}*/
		OVERLAPPEDPIPEREAD overlapped;
		//BOOL bSuccess = ::ReadFileEx(m_hStdOutWrite, m_readPipeBufffer, sizeof(out_buffer), &overlapped, m_readPipeOverlappedCallbackfun);
		BOOL bSuccess = ::ReadFile(m_hStdOutWrite, out_buffer, sizeof(out_buffer), &bytes, NULL);
		if (bSuccess && (dwRead > 0))  
		{
			if (m_readCallbackfun)
			{
				m_readCallbackfun(string(out_buffer),this);
			}
#ifdef _DEBUG
			printf("Subprocess output:%s\r\n", out_buffer);
#endif // _DEBUG
			ZeroMemory(out_buffer, sizeof(out_buffer));
		}
		/*else
		{
			if (m_readCallbackfun)
			{
				m_readCallbackfun(string("Read Pipe fail ,the error code :") + std::to_string(GetLastError()) + "\r\n", this);
			}
		}*/
	}
	return;
}

void subProcessRunner::_isFinished()
{
	::WaitForSingleObject(m_piProcInfo.hProcess, INFINITE);
	m_bisFinished = true;
	if (m_readSubProcessOutputThread && m_readSubProcessOutputThread->joinable())
	{
		m_readSubProcessOutputThread->join();
		m_readSubProcessOutputThread = nullptr;
	}
	if (!m_hStdInRead && !m_hStdOutWrite)
	{
		return;
	}
	::CloseHandle(m_piProcInfo.hThread);
	::CloseHandle(m_piProcInfo.hProcess);
	::CloseHandle(m_hStdInRead);
	::CloseHandle(m_hStdOutWrite);
	m_hStdInRead = NULL;
	m_hStdOutWrite = NULL;
}
