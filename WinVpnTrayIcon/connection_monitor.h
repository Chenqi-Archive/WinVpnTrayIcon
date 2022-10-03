#pragma once

#include "system/win32_vpn.h"

#include <functional>


class ConnectionMonitor : public VpnInfo {
public:
	ConnectionMonitor(VpnInfo&& connection, std::function<void()> callback);
	~ConnectionMonitor();
private:
	static constexpr size_t max_wait_second = 30;
	size_t wait_second = 0;
	std::function<void()> callback;
public:
	void Connect();
	void Disconnect();
};
