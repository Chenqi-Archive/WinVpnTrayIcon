#include "win32_vpn.h"

#include <map>

#include <windows.h>
#include "ras.h"
#include "raserror.h"


#pragma comment(lib, "rasapi32.lib")


namespace {

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


void VpnInfo::Connect() {
	if (IsConnected()) { return; }
	handle = nullptr;
	RASDIALPARAMS ras_entry = {};
	BOOL password_saved;
	ras_entry.dwSize = sizeof(RASDIALPARAMS);
	wcscpy_s(ras_entry.szEntryName, name.c_str());
	RasGetEntryDialParams(nullptr, &ras_entry, &password_saved);
	RasDial(nullptr, nullptr, &ras_entry, 0, nullptr, (HRASCONN*)&handle);
}

void VpnInfo::Check() {
	if (handle == nullptr) { state = State::Disconnected; return; }
	RASCONNSTATUS rasStatus = {};
	rasStatus.dwSize = sizeof(RASCONNSTATUS);
	RasGetConnectStatus((HRASCONN)handle, &rasStatus);
	if (rasStatus.rasconnstate < RASCS_Connected) { state = State::Connecting; return; }
	if (rasStatus.rasconnstate == RASCS_Connected) { state = State::Connected; return; }
	if (rasStatus.rasconnstate == RASCS_Disconnected) { state = State::Disconnected; handle = nullptr; return; }
}

void VpnInfo::Disconnect() {
	if (handle == nullptr) { return; }
	RasHangUp((HRASCONN)handle);
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
