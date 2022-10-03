#include "ConnectionMenu.h"
#include "RefreshButton.h"
#include "ConnectionList.h"
#include "ExitButton.h"

#include "WndDesign/geometry/clamp.h"
#include "WndDesign/frame/PaddingFrame.h"
#include "WndDesign/system/win32_api.h"
#include "WndDesign/window/Global.h"


BEGIN_NAMESPACE(Anonymous)

// style
constexpr ValueTag width = 200px;
constexpr ValueTag height_max = 70pct;

// layout
inline Rect CalculateRegion(Size size, Point point_left_bottom, Size size_ref) {
	return clamp(Rect(point_left_bottom - Vector(0, size.height), size), Rect(point_zero, size_ref));
}

END_NAMESPACE(Anonymous)


ConnectionMenu::ConnectionMenu(ConnectionTrayIcon& tray_icon) : Base{
	L"",
	[&]() {
		alloc_ptr<ConnectionList> connection_list = new ConnectionList(tray_icon);
		return new PaddingFrame{
			Padding(6px, 1px, 1px, 1px),
			new ListLayout<Vertical>{
				1px,
				new RefreshButton(*connection_list),
				connection_list,
				new ExitButton(*this)
			}
		};
	}()
} {
	Win32::SetWndStyleTool(GetHWND());
}

void ConnectionMenu::SetPosition(Point position) {
	DesktopFrameRegionUpdated(CalculateRegion(size, this->position = position, size_ref));
}

std::pair<Size, Size> ConnectionMenu::CalculateMinMaxSize(Size size_ref) {
	return { size, size };
}

Rect ConnectionMenu::OnDesktopFrameSizeRefUpdate(Size size_ref) {
	return CalculateRegion(size = UpdateChildSizeRef(child, Size(width.value(), 0)), position, this->size_ref = size_ref);
}

void ConnectionMenu::OnChildSizeUpdate(WndObject& child, Size child_size) {
	DesktopFrameRegionUpdated(CalculateRegion(size = child_size, position, size_ref));
}

void ConnectionMenu::Hide() { DesktopFrame::Hide(); }

void ConnectionMenu::Exit() { global.Terminate(); }

void ConnectionMenu::OnKeyMsg(KeyMsg msg) {
	switch (msg.key) {
	case Key::Escape: Hide(); break;
	}
}

void ConnectionMenu::OnNotifyMsg(NotifyMsg msg) {
	switch (msg) {
	case NotifyMsg::LoseFocus: Hide(); break;
	}
}

void ConnectionMenu::Show(ConnectionTrayIcon& tray_icon, Point position) {
	static ConnectionMenu& connection_menu = static_cast<ConnectionMenu&>(global.AddWnd(new ConnectionMenu(tray_icon)));
	connection_menu.SetPosition(position);
	connection_menu.DesktopFrame::Show();
	connection_menu.DesktopFrame::SetForeground();
	connection_menu.SetFocus();
}
