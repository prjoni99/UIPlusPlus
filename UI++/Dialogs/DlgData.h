#pragma once

namespace FTW
{
	struct Scale
	{
		Scale(int xx = 96, int yy = 96)
			: x(xx), y(yy) {}

		int x;
		int y;
	};

	typedef short DialogTraitFlags;

	struct DialogTraits
	{
		const static DialogTraitFlags SHOW_ICONS = 1;
		const static DialogTraitFlags SHOW_SIDEBAR = 2;
		const static DialogTraitFlags ALLOW_VARIABLE_EDITOR = 4;
		const static DialogTraitFlags ALLOW_BACK = 8;
		const static DialogTraitFlags ALLOW_REFRESH = 16;
		const static DialogTraitFlags FLAT = 32;
		const static DialogTraitFlags ALWAYS_ON_TOP = 64;

		const static DialogTraitFlags DEFAULT_FLAGS = SHOW_ICONS | ALLOW_VARIABLE_EDITOR | ALLOW_BACK |
			ALWAYS_ON_TOP | SHOW_SIDEBAR;

		DialogTraits(const DialogTraits& dd) :
			title(dd.title), subtitle(dd.subtitle), fontFace(dd.fontFace), hIcon(dd.hIcon), accentColor(dd.accentColor),
			textColor(dd.textColor), screenScale(dd.screenScale), flags(dd.flags) {}

		DialogTraits(PCTSTR ttl, PCTSTR subttl, PCTSTR fface, const HICON icon, const COLORREF aColor,
			const COLORREF tColor, Scale s, short flags) :
			title(ttl), subtitle(subttl), fontFace(fface), hIcon(icon), accentColor(aColor), textColor(tColor),
			screenScale(s), flags(flags) {}

		static DialogTraitFlags BuildDialogTraitFlags(bool showDialogIcons, bool flatDlgs,
			bool dlgAlwaysOnTop, bool allowEditor, bool showSidebar,
			bool allowBack, bool allowRefresh)
		{
			DialogTraitFlags _flags = 0;

			if (showDialogIcons)
				_flags |= SHOW_ICONS;
			if (flatDlgs)
				_flags |= FLAT;
			if (dlgAlwaysOnTop)
				_flags |= ALWAYS_ON_TOP;
			if (allowEditor)
				_flags |= ALLOW_VARIABLE_EDITOR;
			if (showSidebar)
				_flags |= SHOW_SIDEBAR;
			if (allowBack)
				_flags |= ALLOW_BACK;
			if (allowRefresh)
				_flags |= ALLOW_REFRESH;

			return _flags;
		};

		void AllowBack(bool allow)
		{
			if (allow)
				flags |= ALLOW_BACK;
			else
				flags = flags & ~ALLOW_BACK;
		}

		void AllowRefresh(bool allow)
		{
			if (allow)
				flags |= ALLOW_REFRESH;
			else
				flags = flags & ~ALLOW_REFRESH;
		}

		CString title;
		CString subtitle;
		CString fontFace;
		HICON hIcon;
		COLORREF accentColor;
		COLORREF textColor;
		Scale screenScale;
		short flags;
	};
}