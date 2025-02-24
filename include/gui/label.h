#ifndef __GUI_LABEL_H
#define _LABEL_H

#include <gui/widget.h>
#include <types.h>

class Label : public Widget {
private:
    const char* text;

public:
    Label(Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h, const char* text);
    ~Label();

    void SetText(const char* text);
    void Draw(GraphicsContext* gc) override;
};

#endif
