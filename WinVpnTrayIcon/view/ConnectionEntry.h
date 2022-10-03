#pragma once

#include "../system/win32_vpn.h"

#include "control/TextButton.h"


class ConnectionTrayIcon;

class ConnectionEntry : public TextButton {
public:
	ConnectionEntry(ConnectionTrayIcon& tray_icon, VpnInfo&& connection);
private:
	ConnectionTrayIcon& tray_icon;
	VpnInfo connection;
private:
	void SetBackground(Color color) { if (background != color) { background = color; Redraw(region_infinite); } }
	void OnConnectionUpdate();
private:
	virtual void OnClick() override;
};
