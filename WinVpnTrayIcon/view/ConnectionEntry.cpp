#include "ConnectionEntry.h"
#include "ConnectionList.h"

#include "../ConnectionTrayIcon.h"


BEGIN_NAMESPACE(Anonymous)

// style
struct ConnectionEntryTextStyle : TextBlockStyle {
	ConnectionEntryTextStyle() {

	}
};

constexpr Color background_disconnected = Color::Gray;
constexpr Color background_connecting = Color::LightYellow;
constexpr Color background_connected = Color::LightGreen;

END_NAMESPACE(Anonymous)


ConnectionEntry::ConnectionEntry(ConnectionTrayIcon& tray_icon, VpnInfo&& connection) :
	TextButton(ConnectionEntryTextStyle(), connection.GetName()),
	tray_icon(tray_icon),
	connection(std::move(connection)) {
	OnConnectionUpdate();
}

void ConnectionEntry::OnConnectionUpdate() {
	switch (connection.GetState()) {
	case VpnInfo::State::Disconnected:
		SetBackground(background_disconnected);
		break;
	case VpnInfo::State::Connecting:
		SetBackground(background_connecting);
		break;
	case VpnInfo::State::Connected:
		SetBackground(background_connected);
		break;
	}
}

void ConnectionEntry::OnClick() {
	switch (connection.GetState()) {
	case VpnInfo::State::Disconnected:
		connection.Connect([&]() {
			OnConnectionUpdate();
			tray_icon.Refresh();
		});
		break;
	case VpnInfo::State::Connecting:
	case VpnInfo::State::Connected:
		connection.Disconnect();
		OnConnectionUpdate();
		tray_icon.Refresh();
		break;
	}
}
