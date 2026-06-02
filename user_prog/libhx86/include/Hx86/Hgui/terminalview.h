#ifndef TERMINAL_VIEW_H
#define TERMINAL_VIEW_H

#include <Hx86/Hgui/widget.h>

class TerminalView : public Widget {
private:
    const char* text;
    FontSize fontSize;

public:
    TerminalView(Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h, const char* text);
    ~TerminalView();

    bool setText(const char* text);
    bool setSize(FontSize size);
    bool setScrollMeta(int32_t totalLines, int32_t visibleLines, int32_t scrollOffset);
    int32_t consumeScrollAction();
};

#endif  // TERMINAL_VIEW_H
