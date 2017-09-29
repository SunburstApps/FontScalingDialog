#include "CAboutDialog.h"

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
	CAboutDialog dlg;
	dlg.Create(HWND_DESKTOP);
	dlg.SetFont(CAboutDialog::GetMessageBoxFont());
	dlg.CenterWindow();
	dlg.ShowWindow(nCmdShow);

	MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}
