#pragma once

#include <atlbase.h>
#include <atlcore.h>
#include <atlwin.h>
#include <math.h>

template<typename T>
class CFontScalingControl
{
private:
	static SIZE GetFontAutoScaleDimensions(HFONT hFont)
	{
		HDC hDC = CreateCompatibleDC(nullptr);
		HGDIOBJ oldFont = SelectObject(hDC, hFont);

		TEXTMETRIC textMetric;
		ZeroMemory(&textMetric, sizeof(textMetric));
		GetTextMetrics(hDC, &textMetric);

		SIZE scaleFactor;
		scaleFactor.cy = textMetric.tmHeight;
		if ((textMetric.tmPitchAndFamily & 1) != 0)
		{
			LPCTSTR alphabet = _T("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
			int alphabetLength = _tcslen(alphabet);

			SIZE extents;
			GetTextExtentPoint32(hDC, alphabet, alphabetLength, &extents);
			scaleFactor.cx = (LONG)round(extents.cx / alphabetLength);
		}
		else {
			scaleFactor.cx = textMetric.tmAveCharWidth;
		}

		SelectObject(hDC, oldFont);
		DeleteDC(hDC);
		return scaleFactor;
	}

	static RECT GetAndAdjustWindowRect(HWND hWnd, HWND hWndParent)
	{
		RECT rect; ::GetWindowRect(hWnd, &rect);
		if (hWndParent == nullptr) return rect;

		::MapWindowPoints(HWND_DESKTOP, hWndParent, (LPPOINT)&rect, 2);
		return rect;
	}

	static constexpr LPTSTR WndProp_DWPHandle = _T("Sunburst.FontScalingControl.DeferWindowPos");
	static constexpr LPTSTR WndProp_AlreadyScaled = _T("Sunburst.FontScalingControl.AlreadyScaled");
	HFONT oldFont;

	LRESULT OnSetFont(UINT, WPARAM wParam, LPARAM lParam, BOOL& handled)
	{
		T *pT = static_cast<T*>(this);

		HFONT newFont = (HFONT)wParam;
		LOGFONTW logFont; ::GetObject(newFont, sizeof(logFont), &logFont);

		if (oldFont == nullptr)
		{
			oldFont = newFont;
			handled = false;
			return 0;
		}

		SIZE newScaleFactor = GetFontAutoScaleDimensions(newFont);
		SIZE oldScaleFactor = GetFontAutoScaleDimensions(oldFont);

		float fractionX = (float)newScaleFactor.cx / (float)oldScaleFactor.cx;
		float fractionY = (float)newScaleFactor.cy / (float)oldScaleFactor.cy;

		// Since I can't tell here exactly how many child controls will be encountered
		// during the entire recursive traversal, I use a somewhat-large number. RAM
		// is cheap, and this should be enough for most cases. If it isn't, then the
		// HDWP will automatically resize itself to store the extra controls.
		HDWP hDefer = GetProp(pT->m_hWnd, WndProp_DWPHandle);
		if (hDefer == nullptr) hDefer = ::BeginDeferWindowPos(256);

		HANDLE alreadyScaledFlag = GetProp(pT->m_hWnd, WndProp_AlreadyScaled);
		if (alreadyScaledFlag == nullptr)
		{
			RECT clientRect = GetAndAdjustWindowRect(pT->m_hWnd, nullptr);

			LONG top = clientRect.top;
			LONG left = clientRect.left;
			LONG width = clientRect.right - clientRect.left;
			LONG height = clientRect.bottom - clientRect.top;

			top = (LONG)roundf(top * fractionY);
			left = (LONG)roundf(left * fractionX);
			width = (LONG)roundf(width * fractionX);
			height = (LONG)roundf(height * fractionY);

			//hDefer = ::DeferWindowPos(hDefer, pT->m_hWnd, nullptr, left, top, width, height, SWP_NOZORDER | SWP_NOACTIVATE);
			::SetWindowPos(pT->m_hWnd, nullptr, left, top, width, height, SWP_NOZORDER | SWP_NOACTIVATE);
			ATLASSERT(hDefer != nullptr);
		}

		SetProp(pT->m_hWnd, WndProp_AlreadyScaled, (HANDLE)1);
		SetProp(pT->m_hWnd, WndProp_DWPHandle, hDefer);

		for (HWND hChild = ::GetWindow(pT->m_hWnd, GW_CHILD); hChild != nullptr; hChild = ::GetWindow(hChild, GW_HWNDNEXT))
		{
			HANDLE flag = GetProp(hChild, WndProp_AlreadyScaled);
			if (flag != nullptr) continue;

			RECT childRect = GetAndAdjustWindowRect(hChild, pT->m_hWnd);
			LONG top = childRect.top;
			LONG left = childRect.left;
			LONG width = childRect.right - childRect.left;
			LONG height = childRect.bottom - childRect.top;

			top = (LONG)roundf(top * fractionY);
			left = (LONG)roundf(left * fractionX);
			width = (LONG)roundf(width * fractionX);
			height = (LONG)roundf(height * fractionY);

			hDefer = ::DeferWindowPos(hDefer, hChild, nullptr, left, top, width, height, SWP_NOZORDER | SWP_NOACTIVATE);
			::SetWindowPos(hChild, nullptr, left, top, width, height, SWP_NOZORDER | SWP_NOACTIVATE);
			ATLASSERT(hDefer != nullptr);

			SetProp(hChild, WndProp_AlreadyScaled, (HANDLE)1);
			SetProp(hChild, WndProp_DWPHandle, hDefer);

			SendMessage(hChild, WM_SETFONT, wParam, lParam);

			RemoveProp(hChild, WndProp_AlreadyScaled);
			RemoveProp(hChild, WndProp_DWPHandle);
		}

		RemoveProp(pT->m_hWnd, WndProp_AlreadyScaled);
		RemoveProp(pT->m_hWnd, WndProp_DWPHandle);

		::EndDeferWindowPos(hDefer);
		oldFont = newFont;

		handled = false;
		return 0;
	}

public:
	BEGIN_MSG_MAP(CFontScalingControl<T>)
		MESSAGE_HANDLER(WM_SETFONT, OnSetFont)
	END_MSG_MAP();

	// Utility function
	HFONT CreateMessageBoxFont()
	{
		T* pT = static_cast<T*>(this);

		NONCLIENTMETRICS ncm;
		ZeroMemory(&ncm, sizeof(ncm));
		ncm.cbSize = sizeof(ncm);
		SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0, GetDpiForWindow(pT->m_hWnd));

		return CreateFontIndirect(&ncm.lfMessageFont);
	}
};
