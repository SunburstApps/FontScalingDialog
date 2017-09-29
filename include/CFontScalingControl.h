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

			::SetWindowPos(pT->m_hWnd, nullptr, left, top, width, height, SWP_NOZORDER | SWP_NOACTIVATE);
		}

		SetProp(pT->m_hWnd, WndProp_AlreadyScaled, (HANDLE)1);

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

			::SetWindowPos(hChild, nullptr, left, top, width, height, SWP_NOZORDER | SWP_NOACTIVATE);

			SetProp(hChild, WndProp_AlreadyScaled, (HANDLE)1);
			SendMessage(hChild, WM_SETFONT, wParam, lParam);
			RemoveProp(hChild, WndProp_AlreadyScaled);
		}

		RemoveProp(pT->m_hWnd, WndProp_AlreadyScaled);

		oldFont = newFont;
		handled = false;
		return 0;
	}

public:
	BEGIN_MSG_MAP(CFontScalingControl<T>)
		MESSAGE_HANDLER(WM_SETFONT, OnSetFont)
	END_MSG_MAP();

	// Utility function
	static HFONT GetMessageBoxFont()
	{
		static HFONT hFont = nullptr;

		if (hFont == nullptr)
		{
			NONCLIENTMETRICS ncm;
			ZeroMemory(&ncm, sizeof(ncm));
			ncm.cbSize = sizeof(ncm);
			SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);
			hFont = CreateFontIndirect(&ncm.lfMessageFont);
		}

		return hFont;
	}
};
