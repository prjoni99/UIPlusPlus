// XDisplayImage.h  Version 1.3
//
// Author: Hans Dietrich
//         hdietrich@gmail.com
//
// License:
//     This file is Copyright © 2009 Hans Dietrich. All Rights Reserved.
//
//     This source file is the property of Hans Dietrich and is not to be
//     re-distributed by any means whatsoever without the expressed written
//     consent of Hans Dietrich.
//
//     This source code can only be used under the Terms of Use set forth
//     on the Hans Dietrich Software web site. Hans Dietrich Software grants 
//     to you (one software developer) the limited right to use this software.
//
//     This software is provided "as is" with no expressed or implied warranty.
//     Hans Dietrich accepts no liability for any damage or loss of business 
//     that this software may cause.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef XDISPLAYIMAGE2_H
#define XDISPLAYIMAGE2_H

namespace HansDietrich
{

HWND DisplayIconFromResource(HWND hWndParent,
							 HMODULE hResModule,
							 LPCTSTR lpszImageResourceId,
							 UINT nCtrlId,
							 RECT& rect,
							 int nIconWidth,
							 int nIconHeight,
							 BOOL bTransparent,
							 BOOL bSendToBack);

HWND DisplayIconFromResource(HWND hWndParent,
							 HMODULE hResModule,
							 UINT nImageResourceId,
							 UINT nCtrlId,
							 RECT& rect,
							 int nIconWidth,
							 int nIconHeight,
							 BOOL bTransparent,
							 BOOL bSendToBack);

HWND DisplayImageFromResource(HWND hWndParent,
							  HMODULE hResModule,
							  LPCTSTR lpszImageResourceType, 
							  LPCTSTR lpszImageResourceId,
							  UINT nCtrlId,
							  RECT& rect,
							  BOOL bTransparent,
							  BOOL bSendToBack);

HWND DisplayImageFromResource(HWND hWndParent,
							  HMODULE hResModule,
							  UINT nImageResourceType, 
							  UINT nImageResourceId,
							  UINT nCtrlId,
							  RECT& rect,
							  BOOL bTransparent,
							  BOOL bSendToBack);

HWND DisplayImageFromUrl(HWND hWndParent,
						 LPCTSTR lpszImageUrl,
						 UINT nCtrlId,
						 RECT& rect,
						 BOOL bTransparent,
						 BOOL bSendToBack);
}
#endif //XDISPLAYIMAGE2_H
