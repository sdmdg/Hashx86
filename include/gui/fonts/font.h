#ifndef FONT_H
#define FONT_H

#include <types.h>
#include <debug.h>

typedef enum {
    REGULAR = 0x0,
    BOLT = 0x1,
    ITALIC = 0x2,
} FontType;

typedef enum {
    SMALL = 8,
    MEDIUM = 12,
    LARGE = 16,
    XLARGE = 24,
} FontSize;

class Font {
public:
    Font();
    ~Font();

    uint32_t (*font_36x36)[684];
    uint8_t (*font_36x36_config)[4];
    uint8_t fontSize;
    FontType fontType;
    int8_t chrAdvanceX = 0;
    int8_t chrAdvanceY = 0;
    uint8_t chrHeight = 0;

    virtual uint8_t getStringLength(const char* str);
    void FormatString(const char* input, char* output, uint32_t maxWidth);
    void setType(FontType type);
    void setSize(FontSize size);
private:
    virtual void update();
};

#endif // FONT_H
