#pragma once

#include "system/win32_tray_icon.h"

#include <memory>


using WndDesign::MouseMsg;
using WndDesign::Point;

class ConnectionMonitor;


class ConnectionTrayIcon : public TrayIcon {
public:
	ConnectionTrayIcon();
	~ConnectionTrayIcon();
private:
	std::unique_ptr<ConnectionMonitor> monitor;
public:
	void Refresh();
private:
	void OnConnectionUpdate();
	void OnLeftClick();
	void OnRightClick(Point mouse_position);
private:
	virtual void OnMouseMsg(MouseMsg msg) override;
};
