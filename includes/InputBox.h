#include <Windows.h>
#include <string>

#define IDC_INPUT 8888

class InputBox {
public:
	static std::wstring ShowInputBox(const std::wstring& title, const std::wstring& prompt) {
		wchar_t buffer[MAX_PATH] = L"";
		std::wstring input;
		DLGTEMPLATE dlgTemplate = { WS_POPUP | WS_BORDER | WS_SYSMENU | DS_MODALFRAME, 0, 0, 300, 100, 0 };

		HWND hwndDlg = CreateDialogIndirectParamW(GetModuleHandle(NULL), &dlgTemplate, NULL, InputDialogProc, (LPARAM)buffer);
		if (hwndDlg) {
			SetWindowTextW(hwndDlg, title.c_str());

			CreateWindowW(L"STATIC", prompt.c_str(), WS_CHILD | WS_VISIBLE, 10, 10, 280, 20, hwndDlg, NULL, NULL, NULL);
			HWND hwndInput = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 10, 30, 280, 20, hwndDlg, (HMENU)IDC_INPUT, NULL, NULL);
			CreateWindowW(L"BUTTON", L"OK", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 10, 60, 100, 30, hwndDlg, NULL, NULL, NULL);
			CreateWindowW(L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE, 130, 60, 100, 30, hwndDlg, NULL, NULL, NULL);

			ShowWindow(hwndDlg, SW_SHOW);
			UpdateWindow(hwndDlg);

			MSG msg;
			while (GetMessage(&msg, NULL, 0, 0)) {
				if (!IsDialogMessage(hwndDlg, &msg)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
		}

		input = buffer;

		return input;
	}

	static INT_PTR CALLBACK InputDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		switch (uMsg) {
		case WM_INITDIALOG:
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			case IDOK:
				GetDlgItemTextW(hwndDlg, IDC_INPUT, (LPWSTR)lParam, MAX_PATH);
				EndDialog(hwndDlg, IDOK);
				return TRUE;

			case IDCANCEL:
				EndDialog(hwndDlg, IDCANCEL);
				return TRUE;
			}
			break;
		}

		return FALSE;
	}
};
