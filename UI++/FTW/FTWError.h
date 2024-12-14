#pragma once

#include "stdafx.h"
#include "Utils.h"

namespace FTW {

	class FTWException
	{
	public:
		enum class classification {Success = 0, Info, Warning, Error};

		FTWException(classification c = classification::Error, PCTSTR data = 0) : m_classification(c), m_dataMessage(data) {};
		FTWException(PCTSTR data, classification c = classification::Error) : m_classification(c), m_dataMessage(data) {};
		~FTWException() {};
		
		//ConfigMgrFTWError(DWORD ID, type t = Info) : m_errID(ID), m_type(t) { };
		////ConfigMgrFTWError(DWORD ID, _com_error& ce,  type t = Info) : m_errID(ID), m_message(ce.ErrorMessage()), m_type(t) { };
		////ConfigMgrFTWError(_com_error& ce,  type t = Info) : m_errID(0), m_message(ce.ErrorMessage()), m_type(t) { };
		//ConfigMgrFTWError(HRESULT hr, type t = Error) : m_hr(hr), m_type(t) { };

		//ConfigMgrFTWError(const ConfigMgrFTWError& err) { m_hr = err.m_hr; };
		//ConfigMgrFTWError& operator=(const ConfigMgrFTWError& err) {  m_hr = err.m_hr; };

		//inline CString Message(void) 
		//{ 
		//	if (m_errID > 0)
		//		return FormatResourceString(m_errID, m_message); 
		//	//else if (m_errID > 0)
		//	//	return FormatResourceString(m_errID, m_message); 
		//	if (m_hr)
		//		return FormatHRString(m_hr);

		//	return _T("No Error");
		//}; 

		virtual CString Message(void) = 0;
		CString DataMessage(void) { return m_dataMessage; };

		classification GetClassification(void) { return m_classification; };

	protected:
		CString m_message;
		CString m_dataMessage;
		classification m_classification;
	};

	class FTWStdException : public FTWException
	{
	public:
		FTWStdException(int code, PTSTR pMessage) : m_message(pMessage), m_code(code) {};
		FTWStdException() : m_code(0) {};

		CString Message(void)
		{
			return m_message;
		};

		int GetErrorCode(void) { return m_code; }

	protected:
		CString m_message;
		int m_code = 0;
	};

	class FTWErrorCodeException : public FTWException
	{
	public:
		FTWErrorCodeException(HRESULT hr, classification c = classification::Error, PCTSTR data = 0) : FTWException(c, data), m_hr(hr) {};
		FTWErrorCodeException(HRESULT hr, PCTSTR data, classification c = classification::Error) : FTWException(c, data), m_hr(hr) {};
		~FTWErrorCodeException() {};

		CString Message(void)
		{
			m_message = FormatHRString(m_hr).c_str();

			if (m_dataMessage.GetLength() > 0)
			{
				m_message.Append(_T(" ["));
				m_message.Append(m_dataMessage);
				m_message.Append(_T("]"));
			}
			
			return m_message;
		}

		HRESULT GetErrorID(void) { return m_hr; }

	protected:
		HRESULT m_hr;

	};

	class FTWDWORDException : public FTWException
	{
	public:
		FTWDWORDException(DWORD ID, classification c = classification::Error, PCTSTR data = 0) : FTWException(c, data), m_errID(ID) {};
		FTWDWORDException(DWORD ID, PCTSTR data, classification c = classification::Error) : FTWException(c, data), m_errID(ID) {};
		FTWDWORDException(DWORD ID, DWORD errorID, classification c = classification::Error) : FTWException(c, FormatHRString(HRESULT_FROM_WIN32(errorID)).c_str()), m_errID(ID) {};
		~FTWDWORDException() {};

		CString Message(void)
		{
			m_message = FormatResourceString(m_errID).c_str();

			if (m_dataMessage.GetLength() > 0)
			{
				m_message.Append(_T(" ["));
				m_message.Append(m_dataMessage);
				m_message.Append(_T("]"));
			}

			return m_message;
		}

		DWORD GetErrorID(void) { return m_errID; };

	protected:
		DWORD m_errID;

	};
}