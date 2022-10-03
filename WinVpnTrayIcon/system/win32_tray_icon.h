#pragma once

#include "WndDesign/message/mouse_msg.h"

#include <string>
#include <memory>


class TrayIcon {
private:
	friend struct TrayIconApi;

public:
	TrayIcon(std::wstring icon_file, std::wstring tooltip);
	~TrayIcon();

private:
	struct Data;
	std::unique_ptr<Data> data;

public:
	void SetIcon(std::wstring icon_file);
	void SetTooltip(std::wstring tooltip);
	void Update();

private:
	bool is_shown = false;
public:
	bool IsShown() const { return is_shown; }
	void Show();
	void Hide();

protected:
	virtual void OnMouseMsg(WndDesign::MouseMsg msg) {}
};
