#pragma once

#include "ConnectionList.h"

#include "control/TextButton.h"


class RefreshButton : public TextButton {
private:
	struct RefreshButtonTextStyle : TextBlockStyle {
		RefreshButtonTextStyle() {

		}
	};
public:
	RefreshButton(ConnectionList& connection_list) :
		TextButton(RefreshButtonTextStyle(), L"Refresh"),
		connection_list(connection_list) {
	}

	// message
private:
	ConnectionList& connection_list;
private:
	virtual void OnClick() override { connection_list.Refresh(); }
};
