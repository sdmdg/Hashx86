/**
 * @file        terminalview.cpp
 * @brief       TerminalView (part of #x86 GUI Framework)
 *
 * @date        06/03/2026
 * @version     1.0.0
 */

#include <Hx86/Hgui/terminalview.h>

TerminalView::TerminalView(Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h,
                           const char* text)
    : Widget(parent, x, y, w, h), text(text), fontSize(TINY) {
    WidgetData data = {parent->ID, x, y, (uint32_t)w, (uint32_t)h, text};
    this->ID = HguiAPI(TERMINAL_VIEW, CREATE, (void*)&data);
}

TerminalView::~TerminalView() {}

bool TerminalView::setText(const char* text) {
    this->text = text;
    WidgetData data = {ID, 0, 0, 0, 0, text};
    return HguiAPI(TERMINAL_VIEW, SET_TEXT, (void*)&data);
}

bool TerminalView::setSize(FontSize size) {
    this->fontSize = size;
    WidgetData data = {ID, (int32_t)fontSize};
    return HguiAPI(TERMINAL_VIEW, SET_FONT_SIZE, (void*)&data);
}

bool TerminalView::setScrollMeta(int32_t totalLines, int32_t visibleLines, int32_t scrollOffset) {
    WidgetData data = {ID, totalLines, visibleLines, (uint32_t)scrollOffset, 0, nullptr};
    return HguiAPI(TERMINAL_VIEW, SET_SCROLL_META, (void*)&data);
}

int32_t TerminalView::consumeScrollAction() {
    WidgetData data = {ID, 0, 0, 0, 0, nullptr};
    return (int32_t)HguiAPI(TERMINAL_VIEW, GET_SCROLL_ACTION, (void*)&data);
}
