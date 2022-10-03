#include "ConnectionTrayIcon.h"

#include "system/win32_vpn.h"
#include "view/ConnectionMenu.h"


BEGIN_NAMESPACE(Anonymous)

std::wstring icon_connection_off = L"assets/connection_off.ico";
std::wstring icon_connection_on = L"assets/connection_on.ico";

inline std::wstring tooltip_none() { return L"no connection available"; }
inline std::wstring tooltip_connecting(std::wstring name) { return name + L" connecting"; }
inline std::wstring tooltip_connected(std::wstring name) { return name + L" connected"; }
inline std::wstring tooltip_disconnected(std::wstring name) { return name + L" disconnected"; }

END_NAMESPACE(Anonymous)


ConnectionTrayIcon::ConnectionTrayIcon() : TrayIcon(icon_connection_off, tooltip_none()) {
	Refresh();
}

ConnectionTrayIcon::~ConnectionTrayIcon() {}

void ConnectionTrayIcon::Refresh() {
	std::vector<VpnInfo> connection_list = VpnInfo::Enumerate();
	if (connection_list.empty()) {
		SetIcon(icon_connection_off);
		SetTooltip(tooltip_none());
		Update();
		return;
	}
	for (auto& info : connection_list) {
		if (info.GetState() == VpnInfo::State::Connected) {
			connection.reset(new VpnInfo(std::move(info)));
			OnConnectionUpdate();
			return;
		}
	}
	connection.reset(new VpnInfo(std::move(connection_list.front())));
	OnConnectionUpdate();
	return;
}

void ConnectionTrayIcon::OnConnectionUpdate() {
	switch (connection->GetState()) {
	case VpnInfo::State::Disconnected:
		SetIcon(icon_connection_off);
		SetTooltip(tooltip_disconnected(connection->GetName()));
		break;
	case VpnInfo::State::Connecting:
		SetTooltip(tooltip_connecting(connection->GetName()));
		break;
	case VpnInfo::State::Connected:
		SetIcon(icon_connection_on);
		SetTooltip(tooltip_connected(connection->GetName()));
		break;
	}
	Update();
}

void ConnectionTrayIcon::OnLeftClick() {
	if (connection == nullptr) { Refresh(); if (connection == nullptr) { return; } }
	switch (connection->GetState()) {
	case VpnInfo::State::Disconnected:
		connection->Connect([&]() { OnConnectionUpdate(); });
		break;
	case VpnInfo::State::Connecting:
	case VpnInfo::State::Connected:
		connection->Disconnect();
		OnConnectionUpdate();
		break;
	}
}

void ConnectionTrayIcon::OnRightClick(Point mouse_position) {
	ConnectionMenu::Show(*this, mouse_position);
}

void ConnectionTrayIcon::OnMouseMsg(MouseMsg msg) {
	switch (msg.type) {
	case MouseMsg::LeftDown: OnLeftClick(); break;
	case MouseMsg::RightDown: OnRightClick(msg.point); break;
	}
}
