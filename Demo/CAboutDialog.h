#pragma once

#include <atlbase.h>
#include <atlcore.h>
#include <atlwin.h>
#include "../CFontScalingControl.h"
#include "resource.h"

class CAboutDialog :
	public CDialogImpl<CAboutDialog>,
	public CFontScalingControl<CAboutDialog>
{
private:
	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
		HICON hIcon = LoadIcon(_AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE(IDI_DEMO));
		this->SetIcon(hIcon, true);
		this->SetIcon(hIcon, false);

		return (INT_PTR)TRUE;
	}

	LRESULT OnCommand(UINT, WPARAM wParam, LPARAM, BOOL&)
	{
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			this->DestroyWindow();
			return (INT_PTR)TRUE;
		}

		return (INT_PTR)FALSE;
	}

	LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL&)
	{
		PostQuitMessage(0);
		return (INT_PTR)FALSE;
	}
	
	LRESULT OnClose(UINT, WPARAM, LPARAM, BOOL&)
	{
		this->DestroyWindow();
		return (INT_PTR)FALSE;
	}

public:
	enum { IDD = IDD_ABOUTBOX };

	BEGIN_MSG_MAP(CAboutDialog)
		CHAIN_MSG_MAP(CFontScalingControl<CAboutDialog>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_COMMAND, OnCommand)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
	END_MSG_MAP();
};
