#include "ConnectionList.h"
#include "ConnectionEntry.h"

#include "../ConnectionTrayIcon.h"
#include "../connection_monitor.h"

#include "WndDesign/frame/PaddingFrame.h"


ConnectionList::ConnectionList(ConnectionTrayIcon& tray_icon) :
	ListLayout(1px), tray_icon(tray_icon) {
	Refresh();
}

void ConnectionList::Refresh() {
	child_list.clear();
	for (auto& connection : VpnInfo::Enumerate()) {
		auto& item = child_list.emplace_back(
			new PaddingFrame{
				Padding(5px, 0px, 0px, 0px),
				new ConnectionEntry(std::move(connection))
			}
		);
		RegisterChild(item.child);
	}
	UpdateIndex(0);
	for (auto& info : child_list) {
		info.length = UpdateChildSizeRef(info.child, Size(size.width, length_min)).height;
	}
	UpdateLayout(0);
}

void ConnectionList::RefreshTrayIcon() {
	tray_icon.Refresh();
}
