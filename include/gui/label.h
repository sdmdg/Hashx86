#ifndef LABEL_H
#define LABEL_H

#include <gui/widget.h>
#include <types.h>

class Label : public Widget {
private:
    char* text;

public:
    Label(Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h, const char* text);
    ~Label();
    void update();
    void setText(const char* text);
    void setSize(FontSize size);
    void setType(FontType type);
    void RedrawToCache() override;
    void Draw(GraphicsContext* gc) override;
};

#endif
