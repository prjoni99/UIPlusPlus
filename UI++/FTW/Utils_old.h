// Utils.h
// 
// Copyright (c) Jason Sandys, 2009
//
// License: This code is released according to the 
// Microsoft Public License (Ms-PL) as documented at 
// http://UIappchooser.codeplex.com/license
//
// Some code in this file is based on a series of articles by David 
// Sackstein: http://blogs.microsoft.co.il/blogs/davids/archive/2008/12/20/msxml-in-c-but-as-elegant-as-c.aspx
//
//////////////////////////////////////////////////////////////////////

#pragma once

#define MAX_STRING_LENGTH 1024

namespace FTW
{
	
	inline bool CheckForFlag(const short flags, const short flag) {
	return ((flags & flag) == flag);
};

PCTSTR BoolString(bool value);

PCTSTR BoolString(VARIANT_BOOL value);

PCTSTR BoolString(BOOL value);

CString FormatHRString(HRESULT hr);
CString FormatResourceString(DWORD dwStrID, ...);
CString FormatResourceString(DWORD dwStrID, va_list* args);

//DWORD DownloadFileFromURL(CString& destinationPathandName, PCTSTR pURL, PCTSTR pDestination = nullptr, bool binary = false);

COLORREF HexToCOLORREF(PCTSTR hexColor, const COLORREF defaultColor = RGB(0, 0, 0));

BOOL Is64BitOS();

bool IsTrue(PCTSTR value);

bool FileExists(PCTSTR filename); 

CString JoinPath(PCTSTR part1, PCTSTR part2);

CString FormatSeconds(int totalSeconds);
}

#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WFILE__ WIDEN(__FILE__)
//wchar_t *pwsz = __WFILE__;

#ifdef _UNICODE
#define __TFILE__ __WFILE__
#else
#define __TFILE__ __FILE__
#endif