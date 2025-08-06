/**
 * @file        label.cpp
 * @brief       Label (part of #x86 GUI Framework)
 * 
 * @date        10/02/2025
 * @version     1.0.0-beta
 */

#include <gui/label.h>

Label::Label(Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h, const char* text)
    : Widget(parent, x, y, w, h)
{
    this->y += 10;
    this->font = new SegoeUI();
    
    //char* formattedText[512]; // Adjust size as needed
    //font->FormatString(text, *formattedText, w);
    //DEBUG_LOG(*formattedText);
    this->text = new char[strlen(text) + 1];
    strcpy(this->text, text);
}

Label::~Label()
{
    delete[] cache;
    delete[] text;
}

void Label::update()
{
    // Clear the cache
    memset(cache, 0, sizeof(uint32_t) * w * h);

    isDirty = true;
}

void Label::setText(const char* text)
{
    delete[] this->text;
    this->text = new char[strlen(text) + 1];
    strcpy(this->text, text);
    update();
}

void Label::setSize(FontSize size)
{
    this->font->setSize(size);
    DEBUG_LOG("updating size %x", size);
    update();
}

void Label::setType(FontType type)
{
/*     this->font->setType(type);
    update(); */
}

void Label::RedrawToCache()
{
    NINA::activeInstance->DrawString(cache, w, h, 2, 2, text, font, LABEL_TEXT_COLOR_NORMAL);
    isDirty = false;
}


void Label::Draw(GraphicsContext* gc)
{
    Widget::Draw(gc);
}
