#ifndef HLISTVIEW_H
#define HLISTVIEW_H

#include <Hx86/Hgui/widget.h>

struct ListViewItemData {
    char name[64];
    uint32_t size;
    uint8_t type;  // 0 = file, 1 = directory, 2 = executable
};

class HListView : public Widget {
public:
    HListView(Widget* parent, int32_t x, int32_t y, uint32_t w, uint32_t h);
    ~HListView();

    void SetItems(ListViewItemData* items, int count);
    void Clear();
    int GetSelectedIndex();
    void SetHeader(const char* text);
};

#endif  // HLISTVIEW_H
