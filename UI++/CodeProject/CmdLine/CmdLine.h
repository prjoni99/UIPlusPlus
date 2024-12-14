#pragma once

namespace CodeProject {

class CCmdLine 
    : public CMapStringToString
{
    DECLARE_DYNAMIC(CCmdLine)
public:
    CCmdLine(LPCTSTR sCmdLine = NULL, BOOL bCaseSensitive = FALSE);
    virtual ~CCmdLine(void);
    void Parse(LPCTSTR pszCmdLine);
    inline const CString& GetCmdLine(void) const { return m_strCmdLine; }
    inline void SetCaseSensitive(BOOL bSensitive) { m_bCaseSensitive = bSensitive; }
    inline BOOL GetCaseSensitive(void) const { return m_bCaseSensitive; }
    inline BOOL HasKey(LPCTSTR pszKey) const { CString strValue; return CMapStringToString::Lookup(pszKey, strValue); }
private:
    CString m_strCmdLine;
    BOOL m_bCaseSensitive;
    static const TCHAR m_pszDelimeters[];
    static const TCHAR m_pszValueSep[];
    static const TCHAR m_pszQuotes[];
};
}


