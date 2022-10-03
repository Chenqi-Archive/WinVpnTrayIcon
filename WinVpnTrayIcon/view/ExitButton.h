#pragma once

#include "ConnectionMenu.h"

#include "control/TextButton.h"


class ExitButton : public TextButton {
private:
	struct ExitButtonTextStyle : TextBlockStyle {
		ExitButtonTextStyle() {

		}
	};
public:
	ExitButton(ConnectionMenu& connection_menu) :
		TextButton(ExitButtonTextStyle(), L"Exit"),
		connection_menu(connection_menu) {
	}

	// message
private:
	ConnectionMenu& connection_menu;
private:
	virtual void OnClick() override { connection_menu.Exit(); }
};
