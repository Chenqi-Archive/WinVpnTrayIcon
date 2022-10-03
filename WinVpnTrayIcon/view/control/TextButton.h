#pragma once

#include "WndDesign/window/wnd_traits.h"
#include "WndDesign/figure/shape.h"
#include "WndDesign/figure/text_block.h"


using namespace WndDesign;


class TextButton : public WndType<Assigned, Auto> {
public:
	TextButton(TextBlockStyle style, std::wstring text) : style(style), text(text), text_block(style, text) {}

	// style
protected:
	Color background = Color::White;
	Color mask_hover = Color(Color::White, 63);
	Color mask_press = Color(Color::White, 127);

	// text
protected:
	TextBlockStyle style;
	std::wstring text;
	TextBlock text_block;

	// layout
protected:
	virtual Size OnSizeRefUpdate(Size size_ref) override {
		text_block.UpdateSizeRef(Size(size_ref.width, length_max));
		return Size(size_ref.width, text_block.GetSize().height);
	}

	// paint
protected:
	virtual void OnDraw(FigureQueue& figure_queue, Rect draw_region) override {
		figure_queue.add(draw_region.point, new Rectangle(draw_region.size, background));
		figure_queue.add(point_zero, new TextBlockFigure(text_block, style.font._color));
		switch (state) {
		case State::Hover: figure_queue.add(draw_region.point, new Rectangle(draw_region.size, mask_hover)); break;
		case State::Press: figure_queue.add(draw_region.point, new Rectangle(draw_region.size, mask_press)); break;
		}
	}

	// message
private:
	enum class State { Normal, Hover, Press } state = State::Normal;
private:
	void OnStateUpdate() { Redraw(region_infinite); }
private:
	virtual void OnMouseMsg(MouseMsg msg) override {
		switch (msg.type) {
		case MouseMsg::LeftDown: state = State::Press; OnStateUpdate(); break;
		case MouseMsg::LeftUp: if (state == State::Press) { state = State::Hover; OnStateUpdate(); OnClick(); } break;
		}
	}
	virtual void OnNotifyMsg(NotifyMsg msg) override {
		switch (msg) {
		case NotifyMsg::MouseEnter: if (state == State::Normal) { state = State::Hover; OnStateUpdate(); } break;
		case NotifyMsg::MouseLeave: state = State::Normal; OnStateUpdate(); break;
		}
	}
protected:
	virtual void OnClick() {}
};
