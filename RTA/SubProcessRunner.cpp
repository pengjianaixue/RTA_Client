// SubProcessRunner.cpp : 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#include "subProcessRunner.h"
SubProcessRunner::SubProcessRunner()
{

}

bool SubProcessRunner::startRun(const string &subprogrammcmd)
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
	DWORD dwCreationFlags = (GetConsoleWindow() ? 0 : CREATE_NO_WINDOW);
	dwCreationFlags |= CREATE_UNICODE_ENVIRONMENT;
	if (!::CreateProcess(NULL,
		szCommmandLine, &m_saAttr, &m_saAttr, TRUE,
		dwCreationFlags, NULL, NULL,
		&m_siStartInfo, &m_piProcInfo))
	{
		CloseHandle(m_hReadChildStdOutRead);
		CloseHandle(m_hChildStdInWrite);
		m_hReadChildStdOutRead = NULL;
		m_hChildStdInWrite = NULL;
#ifdef _DEBUG
		qDebug()<<("CreateProcess fail ,error code: %d\r\n", GetLastError());
#endif // _DEBUG
		return false;
	}
	
	m_bisFinished = false;
	m_readSubProcessOutputThread = std::make_shared<std::thread>(&SubProcessRunner::readSubProcessOutput,this);
	m_processStatesNotifyThread  = std::make_shared<std::thread>(&SubProcessRunner::_isFinished, this);
	return true;
}

bool SubProcessRunner::isFinished() const
{	
	return m_bisFinished;
}
int SubProcessRunner::waitForFinish()
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
bool SubProcessRunner::registerReadCallBackFuntion(READSTDOUTCALLBACKFUN callBackfunction)
{
	if (callBackfunction)
	{
		m_readCallbackfun = callBackfunction;
		return true;
	}
	return false;
}
bool SubProcessRunner::pause()
{
	if (m_piProcInfo.hThread)
	{
		 int ret = ::SuspendThread(m_piProcInfo.hThread);
		 return !ret;
	}
	return false;
}
bool SubProcessRunner::resume()
{
	if (m_piProcInfo.hThread)
	{
		int ret = ::ResumeThread(m_piProcInfo.hThread);
		return !ret;
	}
	return false;
}
bool SubProcessRunner::stop()
{
	if (m_piProcInfo.hThread && m_piProcInfo.hProcess)
	{
		TerminateThread(m_piProcInfo.hThread,-1);
		TerminateProcess(m_piProcInfo.hProcess, -1);
		return true;
	}
	return false;
}
string SubProcessRunner::wirteCmdtoSubprogramm()
{
	return string();
}

SubProcessRunner::~SubProcessRunner()
{
	if (m_processStatesNotifyThread && m_processStatesNotifyThread->joinable())
	{
		m_processStatesNotifyThread->join();
		m_processStatesNotifyThread = nullptr;
	}
}

void SubProcessRunner::createSecurityAttributes(PSECURITY_ATTRIBUTES pSa)
{
	pSa->bInheritHandle = TRUE;
	pSa->lpSecurityDescriptor = NULL;
	pSa->nLength = sizeof(SECURITY_ATTRIBUTES);
}

bool SubProcessRunner::initMemberVar()
{
	createSecurityAttributes(&m_saAttr);
	if (!::CreatePipe(&m_hReadChildStdOutRead, &m_hChildStdInWrite, &m_saAttr, 0))
		return false;
	SetStdHandle(STD_OUTPUT_HANDLE, m_hChildStdInWrite);
	return true;
}

void SubProcessRunner::createStartUpInfo(LPSTARTUPINFO lpStartUpInfo)
{
	ZeroMemory(lpStartUpInfo,sizeof(STARTUPINFO));
	*lpStartUpInfo = { sizeof(STARTUPINFO), 0, 0, 0,
		(unsigned long)CW_USEDEFAULT, (unsigned long)CW_USEDEFAULT,
		(unsigned long)CW_USEDEFAULT, (unsigned long)CW_USEDEFAULT,
		0, NULL, 0,
		STARTF_USESTDHANDLES,
		0, 0, 0,
		GetStdHandle(STD_INPUT_HANDLE), m_hChildStdInWrite, m_hChildStdInWrite
	};

	//lpStartUpInfo->cb = sizeof(STARTUPINFO);
	//lpStartUpInfo->wShowWindow = SW_HIDE;
	//lpStartUpInfo->dwFlags = STARTF_USESTDHANDLES;
	//lpStartUpInfo->hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	//lpStartUpInfo->hStdOutput = m_hChildStdInWrite;
	//lpStartUpInfo->hStdError = m_hChildStdInWrite;//GetStdHandle(STD_ERROR_HANDLE);

}

void SubProcessRunner::readSubProcessOutput()
{

	char out_buffer[10000] = {0};
	DWORD  dwRead = {0};
	while (!m_bisFinished)
	{

		DWORD bytes = 0;
		BOOL bSuccess = ::ReadFile(m_hReadChildStdOutRead, out_buffer, sizeof(out_buffer), &dwRead, NULL);
		if (bSuccess && (dwRead > 0))  
		{
			if (m_readCallbackfun)
			{
				m_readCallbackfun(string(out_buffer),this);
			}
			emit signal_SendPyStdoutContents(QString(out_buffer));
#ifdef _DEBUG
			qDebug()<<("Subprocess output:%s\r\n", out_buffer);
#endif // _DEBUG
			ZeroMemory(out_buffer, sizeof(out_buffer));
		}
		else
		{
			qDebug() << QString("Read Pipe fail ,the error code :") + std::to_string(GetLastError()).c_str() + "\r\n";
		}
	}
	return;
}

void SubProcessRunner::_isFinished()
{
	::WaitForSingleObject(m_piProcInfo.hProcess, INFINITE);
	m_bisFinished = true;
	if (m_readSubProcessOutputThread && m_readSubProcessOutputThread->joinable())
	{
		m_readSubProcessOutputThread->join();
		m_readSubProcessOutputThread = nullptr;
	}
	if (!m_hReadChildStdOutRead && !m_hChildStdInWrite)
	{
		return;
	}
	::CloseHandle(m_piProcInfo.hThread);
	::CloseHandle(m_piProcInfo.hProcess);
	::CloseHandle(m_hReadChildStdOutRead);
	::CloseHandle(m_hChildStdInWrite);
	m_hReadChildStdOutRead = NULL;
	m_hChildStdInWrite = NULL;
}
