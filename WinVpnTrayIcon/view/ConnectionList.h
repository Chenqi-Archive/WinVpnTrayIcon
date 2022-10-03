#pragma once

#include "WndDesign/layout/ListLayout.h"


using namespace WndDesign;


class ConnectionTrayIcon;

class ConnectionList : public ListLayout<Vertical> {
public:
	ConnectionList(ConnectionTrayIcon& tray_icon);
private:
	ConnectionTrayIcon& tray_icon;
public:
	void Refresh();
};
