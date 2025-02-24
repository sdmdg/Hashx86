#ifndef SegoeUI_H
#define SegoeUI_H

#include <gui/fonts/segoeui_data.h>
#include <types.h>
#include <gui/fonts/font.h>


class SegoeUI : public Font {

public:
    SegoeUI();
    ~SegoeUI();
private:
    void update();
};

#endif // SegoeUI_H
