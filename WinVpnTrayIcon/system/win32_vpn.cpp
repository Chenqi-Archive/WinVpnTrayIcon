#include "win32_vpn.h"

#include <map>
#include <unordered_map>
#include <stdexcept>

#include <windows.h>
#include "ras.h"
#include "raserror.h"


#pragma comment(lib, "rasapi32.lib")


inline VpnInfo::State GetConnectionState(RASCONNSTATE state) {
	if (state < RASCS_Connected) { return VpnInfo::State::Connecting; }
	if (state == RASCS_Connected) { return VpnInfo::State::Connected; }
	return VpnInfo::State::Disconnected;
}

inline void SetVpnInfoState(VpnInfo& vpn_info, VpnInfo::State state) {
	vpn_info.state = state;
}


namespace {

HINSTANCE hInstance = NULL;
static const wchar_t ras_dial_callback_class_name[] = L"RasDialCallbackWindowClass";
HWND ras_dial_callback_hwnd = NULL;
std::unordered_map<DWORD, std::pair<VpnInfo*, std::function<void()>>> ras_dial_callback_map;

void RasDialCallbackSync(DWORD index, RASCONNSTATE state) {
	if (auto it = ras_dial_callback_map.find(index); it != ras_dial_callback_map.end()) {
		auto& [vpn_info, callback] = it->second;
		VpnInfo::State state_new = GetConnectionState(state);
		if (vpn_info->GetState() != state_new) {
			SetVpnInfoState(*vpn_info, state_new);
			callback();
		}
	}
}

LRESULT RasDialCallbackWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
	case WM_RASDIALEVENT: RasDialCallbackSync((DWORD)wparam, (RASCONNSTATE)lparam); return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
};

void RasDialCallbackRegisterClass() {
	WNDCLASSEXW wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEXW);
	wcex.lpfnWndProc = RasDialCallbackWndProc;
	wcex.hInstance = hInstance = GetModuleHandle(NULL);
	wcex.lpszClassName = ras_dial_callback_class_name;
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	ATOM res = RegisterClassExW(&wcex);
	if (res == 0) { throw std::runtime_error("register class error"); }
}

void RasDialCallbackCreateWindow() {
	RasDialCallbackRegisterClass();
	ras_dial_callback_hwnd = CreateWindowEx(NULL, ras_dial_callback_class_name, L"", WS_POPUP,
											CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
											NULL, NULL, hInstance, NULL);
	if (ras_dial_callback_hwnd == NULL) { throw std::runtime_error("create window error"); }
}

void RasDialCallbackAsync(DWORD index, DWORD, HRASCONN, UINT, RASCONNSTATE state, DWORD, DWORD) {
	PostMessage(ras_dial_callback_hwnd, WM_RASDIALEVENT, index, state);
}

void AddRasDialCallback(DWORD index, VpnInfo* vpn_info, std::function<void()> callback) {
	static bool is_ras_dial_callback_hwnd_created = false;
	if (!is_ras_dial_callback_hwnd_created) {
		RasDialCallbackCreateWindow();
		is_ras_dial_callback_hwnd_created = true;
	}
	auto [it, success] = ras_dial_callback_map.emplace(index, std::make_pair(vpn_info, callback));
	if (!success) { it->second = { vpn_info, callback }; }
}

void RemoveRasDialCallback(DWORD index) {
	ras_dial_callback_map.erase(index);
}


std::vector<std::wstring> EnumerateVpnName() {
	DWORD ret = 0, size = 0, ras_entry_count = 0;
	ret = RasEnumEntries(nullptr, nullptr, nullptr, &size, &ras_entry_count);
	if (ret != ERROR_BUFFER_TOO_SMALL) { return {}; }

	std::vector<RASENTRYNAME> ras_entry_list(ras_entry_count); ras_entry_list[0].dwSize = sizeof(RASENTRYNAME);
	ret = RasEnumEntries(nullptr, nullptr, ras_entry_list.data(), &size, &ras_entry_count);
	if (ret != ERROR_SUCCESS) { return {}; }

	std::vector<std::wstring> vpn_list; vpn_list.reserve(ras_entry_count);
	for (auto& ras_entry : ras_entry_list) {
		vpn_list.emplace_back(ras_entry.szEntryName);
	}

	return vpn_list;
}

std::map<std::wstring, void*> EnumerateVpnConnection() {
	DWORD ret = 0, size = 0, ras_connection_count = 0;
	ret = RasEnumConnections(nullptr, &size, &ras_connection_count);
	if (ret != ERROR_BUFFER_TOO_SMALL) { return {}; }

	std::vector<RASCONN> ras_connection_list(ras_connection_count); ras_connection_list[0].dwSize = sizeof(RASCONN);
	ret = RasEnumConnections(ras_connection_list.data(), &size, &ras_connection_count);
	if (ret != ERROR_SUCCESS) { return {}; }

	std::map<std::wstring, void*> vpn_connected_list;
	for (auto& ras_connection : ras_connection_list) {
		vpn_connected_list.emplace(ras_connection.szEntryName, ras_connection.hrasconn);
	}
	return vpn_connected_list;
}

}


void VpnInfo::Connect(std::function<void()> callback) {
	if (handle != nullptr) { return; }
	RASDIALPARAMS ras_entry = {};
	BOOL password_saved;
	ras_entry.dwSize = sizeof(RASDIALPARAMS);
	ras_entry.dwCallbackId = (DWORD)this;
	wcscpy_s(ras_entry.szEntryName, name.c_str());
	AddRasDialCallback(ras_entry.dwCallbackId, this, callback);
	RasGetEntryDialParams(nullptr, &ras_entry, &password_saved);
	RasDial(nullptr, nullptr, &ras_entry, 2, RasDialCallbackAsync, (HRASCONN*)&handle);
}

void VpnInfo::Disconnect() {
	if (handle != nullptr) {
		RasHangUp((HRASCONN)handle);
		handle = nullptr;
		state = State::Disconnected();
		RemoveRasDialCallback((DWORD)this);
	}
}

std::vector<VpnInfo> VpnInfo::Enumerate() {
	std::vector<VpnInfo> connection_list;
	std::map<std::wstring, void*> map = EnumerateVpnConnection();
	for (auto& vpn_name : EnumerateVpnName()) {
		if (auto it = map.find(vpn_name); it != map.end()) {
			connection_list.emplace_back(VpnInfo(vpn_name, it->second));
		} else {
			connection_list.emplace_back(VpnInfo(vpn_name));
		}
	}
	return connection_list;
}
