#include "StdAfx.h"
#include "CmdLine.h"

namespace CodeProject
{

IMPLEMENT_DYNAMIC(CCmdLine, CMapStringToString)
 
const TCHAR CCmdLine::m_pszDelimeters[] = _T("-/");
const TCHAR CCmdLine::m_pszQuotes[] = _T("\"\'`");
const TCHAR CCmdLine::m_pszValueSep[] = _T(" :="); // Space MUST be in set, but is not a name/value delimiter.

CCmdLine::CCmdLine(LPCTSTR pszCmdLine, BOOL bCaseSensitive)
    : m_bCaseSensitive(bCaseSensitive)
{
    if (pszCmdLine != NULL) 
    {
        Parse(pszCmdLine);
    }
}
 
CCmdLine::~CCmdLine()
{
 
}
 
void CCmdLine::Parse(LPCTSTR pszCmdLine) 
{
    if (! pszCmdLine)
        return;
    
    m_strCmdLine = pszCmdLine;
    RemoveAll();
 
    int nArgs = 0;
    LPCTSTR pszCurrent = pszCmdLine;
    while(TRUE) 
    {
        // /Key:"arg"
        if (_tcslen(pszCurrent) == 0) 
        { 
            // No data left
            break; 
        } 
 
        LPCTSTR pszArg = _tcspbrk(pszCurrent, m_pszDelimeters);
        if (! pszArg) 
        {
            // No delimeters found
            break; 
        }
 
        LPCTSTR pszInArg;
        do
        {
            pszArg = _tcsinc(pszArg);
            pszInArg = _tcspbrk(pszArg, m_pszDelimeters);
        } while (pszInArg == pszArg);
    
        // Key:"arg"
        if (_tcslen(pszArg) == 0) 
        {
            // String ends with delimeter
            break; 
        }
 
        LPCTSTR pszVal = _tcspbrk(pszArg, m_pszValueSep);
        if (pszVal == NULL) 
        { 
            // Key ends command line
            CString strKey(pszArg);
            if(! m_bCaseSensitive) 
            {
                strKey.MakeLower();
            }
            SetAt(strKey, _T(""));
            break;
        } 
        else if (pszVal[0] == _T(' ') || _tcslen(pszVal) == 1 ) 
        { 
            // Key with no value or cmdline ends with /Key:
            CString strKey(pszArg, (int) (pszVal - pszArg));
            if(! strKey.IsEmpty()) 
            { 
                // Prevent /: case
                if(!m_bCaseSensitive) 
                {
                    strKey.MakeLower();
                }
                SetAt(strKey, _T(""));
            }
 
            pszCurrent = _tcsinc(pszVal);
            continue;
        } 
        else 
        { 
            // Key with value
            CString strKey(pszArg, (int) (pszVal - pszArg));
            if (! m_bCaseSensitive) 
            {
                strKey.MakeLower();
            }
 
            pszVal = _tcsinc(pszVal);
            // "arg"
            LPCTSTR pszQuote = _tcspbrk(pszVal, m_pszQuotes), pszEndQuote(NULL);
            
            if (pszQuote == pszVal) 
            { 
                // Quoted String
                pszQuote = _tcsinc(pszVal);
                pszEndQuote = _tcspbrk(pszQuote, m_pszQuotes);
            } 
            else 
            {
                pszQuote = pszVal;
                pszEndQuote = _tcschr(pszQuote, _T(' '));
            }
 
            if (pszEndQuote == NULL) 
            { 
                // No end quotes or terminating space, take rest of string
                CString strValue(pszQuote);
                if(! strKey.IsEmpty()) 
                { 
                    // Prevent /:val case
                    SetAt(strKey, strValue);
                }
                break;
            } 
            else 
            { 
                // End quote or space present
                if(! strKey.IsEmpty()) 
                {    
                    // Prevent /:"val" case
                    CString strValue(pszQuote, (int) (pszEndQuote - pszQuote));
                    SetAt(strKey, strValue);
                }
                pszCurrent = _tcsinc(pszEndQuote);
                continue;
            }
        }
    }
}

}