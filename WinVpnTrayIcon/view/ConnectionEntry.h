#pragma once

#include "../connection_monitor.h"

#include "control/TextButton.h"


class ConnectionEntry : public TextButton {
public:
	ConnectionEntry(VpnInfo&& connection);
private:
	ConnectionMonitor monitor;
private:
	void SetBackground(Color color) { background = color; Redraw(region_infinite); }
	void OnConnectionUpdate();
private:
	virtual void OnClick() override;
};
