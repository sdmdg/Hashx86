/**
 * @file        listview.cpp
 * @brief       ListView userspace proxy (part of #x86 GUI Framework)
 *
 * @date        22/02/2026
 * @version     1.0.0
 */

#include <Hx86/Hgui/listview.h>

HListView::HListView(Widget* parent, int32_t x, int32_t y, uint32_t w, uint32_t h)
    : Widget(parent, x, y, w, h) {
    WidgetData data = {parent->ID, x, y, w, h, nullptr};
    this->ID = HguiAPI(LISTVIEW, CREATE, (void*)&data);
}

HListView::~HListView() {}

void HListView::SetItems(ListViewItemData* items, int count) {
    WidgetData data = {this->ID, (uint32_t)count, 0, 0, 0, (const char*)items};
    HguiAPI(LISTVIEW, SET_ITEMS, (void*)&data);
}

void HListView::Clear() {
    WidgetData data = {this->ID, 0, 0, 0, 0, nullptr};
    HguiAPI(LISTVIEW, CLEAR_ITEMS, (void*)&data);
}

int HListView::GetSelectedIndex() {
    WidgetData data = {this->ID, 0, 0, 0, 0, nullptr};
    return (int)HguiAPI(LISTVIEW, GET_SELECTED, (void*)&data);
}

void HListView::SetHeader(const char* text) {
    WidgetData data = {this->ID, 0, 0, 0, 0, text};
    HguiAPI(LISTVIEW, SET_TEXT, (void*)&data);
}
