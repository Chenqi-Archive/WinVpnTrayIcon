#include "ConnectionEntry.h"
#include "ConnectionList.h"


BEGIN_NAMESPACE(Anonymous)

// style
struct ConnectionEntryTextStyle : TextBlockStyle {
	ConnectionEntryTextStyle() {

	}
};

constexpr Color background_disconnected = Color::Gray;
constexpr Color background_connecting = Color::Orange;
constexpr Color background_connected = Color::LightGreen;
constexpr Color background_disconnecting = Color::Orange;

END_NAMESPACE(Anonymous)


ConnectionEntry::ConnectionEntry(VpnInfo&& connection) :
	TextButton(ConnectionEntryTextStyle(), connection.GetName()),
	monitor(std::move(connection), [&]() {
	OnConnectionUpdate();
	static_cast<ConnectionList&>(GetParent()).RefreshTrayIcon();
}) {
	OnConnectionUpdate();
}

void ConnectionEntry::OnConnectionUpdate() {
	if (monitor.IsConnected()) {
		SetBackground(background_connected);
	} else {
		SetBackground(background_disconnected);
	}
}

void ConnectionEntry::OnClick() {
	if (monitor.IsDisconnected()) {
		monitor.Connect();
		SetBackground(background_connecting);
	} else {
		monitor.Disconnect();
		SetBackground(background_disconnecting);
	}
}
