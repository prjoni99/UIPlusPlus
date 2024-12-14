// CXWaitCursor.h  Version 1.0
//
// Author:  Hans Dietrich
//          hdietrich@gmail.com
//
// History
//     Version 1.0 - 2009 May 12
//     - Initial release
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
// Public Members:
//           NAME                             DESCRIPTION
//   ---------------------   -------------------------------------------------
//   Construction
//      CXWaitCursor()       Ctor
//
///////////////////////////////////////////////////////////////////////////////

#ifndef CXWAITCURSOR_H
#define CXWAITCURSOR_H

#pragma message("including CXWaitCursor.h")

#include <crtdbg.h>

#pragma warning(push)
#pragma warning(disable : 4127)	// for _ASSERTE: conditional expression is constant

namespace HansDietrich {

	class CXWaitCursor
	{
	public:
		CXWaitCursor() :
			m_nWaitCursorCount(0),
			m_hcurWaitCursorRestore(0)
		{
			DoWaitCursor(1);
		}
		~CXWaitCursor() { DoWaitCursor(-1); }

	private:
		void DoWaitCursor(int nCode)
		{
			// 0 => restore, 1 => begin, -1 => end
			_ASSERTE(nCode == 0 || nCode == 1 || nCode == -1);
			m_nWaitCursorCount += nCode;
			if (m_nWaitCursorCount > 0)
			{
				HCURSOR hcurPrev = ::SetCursor(::LoadCursor(NULL, IDC_WAIT));
				if (nCode > 0 && m_nWaitCursorCount == 1)
					m_hcurWaitCursorRestore = hcurPrev;
			}
			else
			{
				// turn everything off
				m_nWaitCursorCount = 0;     // prevent underflow
				::SetCursor(m_hcurWaitCursorRestore);
			}
		}

		int		m_nWaitCursorCount;
		HCURSOR	m_hcurWaitCursorRestore;
	};
}
#pragma warning(pop)

#endif //CXWAITCURSOR_H
