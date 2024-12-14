#include "StdAfx.h"
#include "Actions.h"
#include "resource.h"
#include "TSVar.h"
#include "Dialogs\DlgProgress.h"

namespace UIpp {

	struct ExternalProcessThreadData
	{
		ExternalProcessThreadData(CString* pCmdLine, CString* exitCodeVar, HWND hDlg, FTWCMLOG::ICMLogPtr pLog, DWORD timeout = XML_ATTRIBUTE_MAXRUNTIME_DEF)
			: cmdLine(pCmdLine), exitCodeVarName(exitCodeVar),
			processTimeout(timeout),
			pCMLog(pLog),
			hProgressDialog(hDlg)
		{}

		const CString* cmdLine;
		const CString* exitCodeVarName;
		DWORD processTimeout;
		HWND hProgressDialog;
		FTWCMLOG::ICMLogPtr pCMLog;
	};

	UINT CallExternalProcess(LPVOID pData)
	{
		ExternalProcessThreadData* pThreadData = static_cast<ExternalProcessThreadData*>(pData);

		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		DWORD exitCode = 0;
		CString commandLine (*(pThreadData->cmdLine));

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;

		pThreadData->pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
			AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			FTW::FormatResourceString(IDS_LOGMSG_RUNEXTERNAL, commandLine, pThreadData->processTimeout));

		if (CreateProcess(NULL,   // No module name (use command line)
			commandLine.GetBuffer() ,        // Command line
			NULL,           // Process handle not inheritable
			NULL,           // Thread handle not inheritable
			FALSE,          // Set handle inheritance to FALSE
			0,              // No creation flags
			NULL,           // Use parent's environment block
			NULL,           // Use parent's starting directory 
			&si,            // Pointer to STARTUPINFO structure
			&pi))
		{

			// Wait until child process exits or the timeout is reached.
			WaitForSingleObject(pi.hProcess, (pThreadData->processTimeout * 1000));

			GetExitCodeProcess(pi.hProcess, &exitCode);

			if(exitCode == STILL_ACTIVE)
			{
				TerminateProcess(pi.hProcess, 1);

				pThreadData->pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error, 
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGERROR_RUNEXTERNAL));

			}
			else
			{
				pThreadData->pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGMG_RUNEXTERNAL_COMPLETE, exitCode));
			}

			if (pThreadData->exitCodeVarName->GetLength() > 0)
				CTSEnv::Instance().Set(pThreadData->pCMLog, *(pThreadData->exitCodeVarName), exitCode);

			::PostMessage(pThreadData->hProgressDialog, WM_CLOSEPROGRESSBAR, (WPARAM)0, (LPARAM)0);

			// Close process and thread handles. 
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);

		}
		else
		{
			pThreadData->pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error, 
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGERROR_EXTERNALPROCESS_FAIL, FTW::FormatHRString(GetLastError()).c_str()));
		}

		return ERROR_SUCCESS;
	}

	INT_PTR CActionExternalCall::Go(void)
	{
		CString commandLine = CTSEnv::Instance().VariableSubstitute(m_actionData.pActionNode->child_value());
		CString progressTitle = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_TITLE);
		CString exitCodeVariable = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_EXITCODEVAR);
		int maxRunTime = GetXMLIntAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_MAXRUNTIME, XML_ATTRIBUTE_MAXRUNTIME_DEF);

		commandLine = commandLine.Trim();

		if (commandLine.GetLength() > 0)
		{
			CDlgUIProgress* pDlg = new CDlgUIProgress(0, progressTitle, m_actionData.globalDialogTraits.fontFace);

			pDlg->Create(IDD_UIPROGRESSDLG);

			pDlg->ShowWindow(SW_SHOW);

			ExternalProcessThreadData threadData(&commandLine, &exitCodeVariable, pDlg->GetSafeHwnd(), m_actionData.pCMLog, maxRunTime);
			
			CWinThread* thread = AfxBeginThread(CallExternalProcess, static_cast<LPVOID>(&threadData));

			if (thread != NULL)
			{
				int returnValue = pDlg->RunModalLoop();
			}

			delete pDlg;
			
			if(thread == NULL)
				throw (FTW::FTWDWORDException(IDS_LOGERROR_THREADCREATEFAIL, GetLastError()));

		}

		return ERROR_SUCCESS;
	}
}