#pragma once

#include "WndDesign/frame/DesktopFrame.h"
#include "WndDesign/wrapper/Background.h"


using namespace WndDesign;


class ConnectionTrayIcon;

class ConnectionMenu : SolidColorBackground<DesktopFrame> {
private:
	ConnectionMenu(ConnectionTrayIcon& tray_icon);

	// layout
private:
	Point position;
	Size size;
	Size size_ref;
private:
	void SetPosition(Point position);
private:
	virtual std::pair<Size, Size> CalculateMinMaxSize(Size size_ref) override;
	virtual Rect OnDesktopFrameSizeRefUpdate(Size size_ref) override;
	virtual void OnChildSizeUpdate(WndObject& child, Size child_size) override;

	// message
public:
	void Hide();
	void Exit();
private:
	virtual void OnKeyMsg(KeyMsg msg) override;
	virtual void OnNotifyMsg(NotifyMsg msg) override;
public:
	static void Show(ConnectionTrayIcon& tray_icon, Point position);
};
