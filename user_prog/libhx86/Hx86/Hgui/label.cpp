/**
 * @file        label.cpp
 * @brief       Label (part of #x86 GUI Framework)
 *
 * @date        10/02/2025
 * @version     1.0.0-beta
 */

#include <Hx86/Hgui/label.h>

Label::Label(Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h, const char* text)
    : Widget(parent, x, y, w, h), text(text) {
    WidgetData data = {parent->ID, x, y, w, h, text};
    this->ID = HguiAPI(LABEL, CREATE, (void*)&data);
}

Label::~Label() {}

bool Label::setText(const char* text) {
    this->text = text;
    WidgetData data = {ID, 0, 0, 0, 0, text};
    return HguiAPI(LABEL, SET_TEXT, (void*)&data);
}

bool Label::setSize(FontSize size) {
    this->fontSize = size;
    WidgetData data = {ID, (uint32_t)fontSize};
    return HguiAPI(LABEL, SET_FONT_SIZE, (void*)&data);
}

bool Label::setType(FontType type) {}
