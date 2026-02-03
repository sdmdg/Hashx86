#ifndef LABEL_H
#define LABEL_H

#include <Hx86/Hgui/widget.h>

class Label : public Widget {
private:
    const char* text;
    FontSize fontSize;

public:
    Label(Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h, const char* text);
    ~Label();

    bool setText(const char* text);
    bool setSize(FontSize size);
    bool setType(FontType type);
};

#endif
