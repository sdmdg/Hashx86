#ifndef INPUTBOX_H
#define INPUTBOX_H

#include <gui/widget.h>
#include <types.h>

class InputBox : public Widget {
private:
    char* text;
    uint32_t capacity;   // max buffer size
    uint32_t length;     // current text length
    uint32_t cursorPos;  // cursor index

public:
    InputBox(Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t capacity = 256);
    ~InputBox();

    void update();
    void setText(const char* newText);
    const char* getText() const { return text; }

    void setSize(FontSize size);
    void setType(FontType type);

    void RedrawToCache() override;
    void Draw(GraphicsDriver* gc) override;

    void OnKeyDown(const char* key) override;
    void OnKeyUp(const char* key) override;
};

#endif // INPUTBOX_H
