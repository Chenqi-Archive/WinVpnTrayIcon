#include "ConnectionTrayIcon.h"
#include "connection_monitor.h"

#include "view/ConnectionMenu.h"


BEGIN_NAMESPACE(Anonymous)

std::wstring icon_connection_off = L"assets/connection_off.ico";
std::wstring icon_connection_on = L"assets/connection_on.ico";

inline std::wstring tooltip_none() { return L"no connection available"; }
inline std::wstring tooltip_connecting(std::wstring name) { return name + L" connecting"; }
inline std::wstring tooltip_connected(std::wstring name) { return name + L" connected"; }
inline std::wstring tooltip_disconnecting(std::wstring name) { return name + L" disconnecting"; }
inline std::wstring tooltip_disconnected(std::wstring name) { return name + L" disconnected"; }

END_NAMESPACE(Anonymous)


ConnectionTrayIcon::ConnectionTrayIcon() : TrayIcon(icon_connection_off, tooltip_none()) {
	Refresh();
	if (monitor != nullptr) { OnConnectionUpdate(); }
}

ConnectionTrayIcon::~ConnectionTrayIcon() {}

void ConnectionTrayIcon::Refresh() {
	std::vector<VpnInfo> connection_list = ConnectionMonitor::Enumerate();
	if (connection_list.empty()) { return; }
	for (auto& info : connection_list) {
		if (info.IsConnected()) {
			monitor.reset(new ConnectionMonitor(std::move(info), [&]() { OnConnectionUpdate(); }));
			return;
		}
	}
	monitor.reset(new ConnectionMonitor(std::move(connection_list.front()), [&]() { OnConnectionUpdate(); }));
}

void ConnectionTrayIcon::OnConnectionUpdate() {
	if (monitor->IsConnected()) {
		SetIcon(icon_connection_on);
		SetTooltip(tooltip_connected(monitor->GetName()));
		Update();
	} else {
		SetIcon(icon_connection_off);
		SetTooltip(tooltip_disconnected(monitor->GetName()));
		Update();
	}
}

void ConnectionTrayIcon::OnLeftClick() {
	if (monitor == nullptr) { Refresh(); if (monitor == nullptr) { return; } }
	if (monitor->IsDisconnected()) {
		monitor->Connect();
		SetTooltip(tooltip_connecting(monitor->GetName()));
		Update();
	} else {
		monitor->Disconnect();
		SetTooltip(tooltip_disconnecting(monitor->GetName()));
		Update();
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
