#include "win32_tray_icon.h"

#include "WndDesign/system/win32_api.h"

#include <Windows.h>


struct TrayIconApi : TrayIcon {
	TrayIcon::OnMouseMsg;
};


BEGIN_NAMESPACE(Anonymous)

using WndDesign::MouseMsg;

static const wchar_t tray_icon_class_name[] = L"TrayIconWindowClass";
HINSTANCE hInstance = NULL;


inline HICON LoadIconFromFile(std::wstring icon_file) {
	HANDLE icon = LoadImage(NULL, icon_file.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
	if (icon == NULL) { throw std::runtime_error("icon file load error"); }
	return (HICON)icon;
}

inline void SetWndUserData(HWND hwnd, void* data) {
	SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)data);
}


LRESULT TrayIconWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	static UINT WM_TASKBAR_CREATE = RegisterWindowMessage(L"TaskbarCreated");
	static auto IsMouseMsg = [](UINT msg) -> bool { return WM_MOUSEFIRST <= msg && msg <= WM_MOUSELAST; };

	TrayIconApi* tray_icon = reinterpret_cast<TrayIconApi*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	if (tray_icon == nullptr) { goto TrayIconIrrelevantMessages; }

	if (tray_icon->IsShown() && msg == WM_TASKBAR_CREATE) { tray_icon->Show(); }

	switch (msg) {
	case WM_USER:
		if (IsMouseMsg(LOWORD(lparam))) {
			MouseMsg mouse_msg;
			switch (LOWORD(lparam)) {
			case WM_LBUTTONDOWN: mouse_msg.type = MouseMsg::LeftDown; break;
			case WM_LBUTTONUP: mouse_msg.type = MouseMsg::LeftUp; break;
			case WM_RBUTTONDOWN: mouse_msg.type = MouseMsg::RightDown; break;
			case WM_RBUTTONUP: mouse_msg.type = MouseMsg::RightUp; break;
			case WM_MBUTTONDOWN: mouse_msg.type = MouseMsg::MiddleDown; break;
			case WM_MBUTTONUP: mouse_msg.type = MouseMsg::MiddleUp; break;
			default: return 0;
			}
			mouse_msg.point = WndDesign::Win32::GetCursorPosWithWndDpi(hwnd);
			tray_icon->OnMouseMsg(mouse_msg);
		}
		return 0;
	}

TrayIconIrrelevantMessages:
	return DefWindowProc(hwnd, msg, wparam, lparam);
};

inline void TrayIconRegisterClass() {
	static bool is_wnd_class_registered = false;
	if (!is_wnd_class_registered) {
		WNDCLASSEXW wcex = {};
		wcex.cbSize = sizeof(WNDCLASSEXW);
		wcex.lpfnWndProc = TrayIconWndProc;
		wcex.hInstance = hInstance = GetModuleHandle(NULL);
		wcex.lpszClassName = tray_icon_class_name;
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		ATOM res = RegisterClassExW(&wcex);
		if (res == 0) { throw std::runtime_error("register class error"); }
		is_wnd_class_registered = true;
	}
}

HWND TrayIconCreateWindow() {
	TrayIconRegisterClass();
	HWND hwnd = CreateWindowEx(NULL, tray_icon_class_name, L"", WS_POPUP,
							   CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
							   NULL, NULL, hInstance, NULL);
	if (hwnd == NULL) { throw std::runtime_error("create window error"); }
	return hwnd;
}


END_NAMESPACE(Anonymous)


struct TrayIcon::Data : NOTIFYICONDATA {
	Data(std::wstring icon_file, std::wstring tooltip) {
		cbSize = sizeof(NOTIFYICONDATA);
		hWnd = TrayIconCreateWindow();
		uID = 0;
		uCallbackMessage = WM_USER;
		hIcon = LoadIconFromFile(icon_file);
		wcscpy_s(szTip, tooltip.c_str());
		uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	}
	~Data() {
		DestroyWindow(hWnd);
	}
};


TrayIcon::TrayIcon(std::wstring icon_file, std::wstring tooltip) : data(new Data(icon_file, tooltip)) {
	SetWndUserData(data->hWnd, this);
}

TrayIcon::~TrayIcon() {
	if (is_shown) { Hide(); }
	SetWndUserData(data->hWnd, nullptr);
}

void TrayIcon::SetIcon(std::wstring icon_file) {
	DestroyIcon(data->hIcon);
	data->hIcon = LoadIconFromFile(icon_file);
}

void TrayIcon::SetTooltip(std::wstring tooltip) {
	wcscpy_s(data->szTip, tooltip.c_str());
}

void TrayIcon::Update() {
	if (is_shown) {
		Shell_NotifyIcon(NIM_MODIFY, data.get());
	}
}

void TrayIcon::Show() {
	Shell_NotifyIcon(NIM_ADD, data.get());
	is_shown = true;
	MSG Msg;
	while (GetMessage(&Msg, NULL, 0, 0)) {
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
}

void TrayIcon::Hide() {
	Shell_NotifyIcon(NIM_DELETE, data.get());
	is_shown = false;
	PostQuitMessage(0);
}
