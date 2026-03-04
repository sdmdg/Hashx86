#ifndef LISTVIEW_H
#define LISTVIEW_H

#include <gui/config/config.h>
#include <gui/widget.h>
#include <types.h>

#define LISTVIEW_MAX_ITEMS 64
#define LISTVIEW_ITEM_HEIGHT 18
#define LISTVIEW_HEADER_HEIGHT 22
#define LISTVIEW_SCROLLBAR_WIDTH 8

struct ListViewItem {
    char name[64];
    uint32_t size;
    uint8_t type;  // 0 = file, 1 = directory, 2 = executable
    bool valid;
};

class ListView : public Widget {
private:
    ListViewItem items[LISTVIEW_MAX_ITEMS];
    int itemCount;
    int scrollOffset;
    int selectedIndex;
    int hoveredIndex;
    char headerText[32];

public:
    ListView(Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h);
    ~ListView();

    void Clear();
    void AddItem(const char* name, uint32_t size, uint8_t type);
    void SetHeader(const char* text);
    int GetSelectedIndex() const {
        return selectedIndex;
    }
    const ListViewItem* GetItem(int index) const;
    int GetItemCount() const {
        return itemCount;
    }

    void update();
    void RedrawToCache() override;

    void OnMouseDown(int32_t x, int32_t y, uint8_t button) override;
    void OnMouseUp(int32_t x, int32_t y, uint8_t button) override;
    void OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy) override;
};

#endif  // LISTVIEW_H
