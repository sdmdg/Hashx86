/**
 * @file        font.cpp
 * @brief       Font (part of #x86 GUI Framework)
 * 
 * @date        12/02/2025
 * @version     1.0.0-beta
 */

#include <gui/fonts/font.h>

Font::Font() {}

Font::~Font() {}

uint8_t Font::getStringLength(const char* str) {
    uint32_t length = 0;
    while (*str) {
        if (*str >= 32 && *str <= 126) {
            uint8_t leftTrim  = this->font_36x36_config[*str - 32][0];
            uint8_t rightTrim = this->font_36x36_config[*str - 32][1];
            length += 36 + 2 - (leftTrim + rightTrim);
        }
        ++str;
    }
    return length;
}


void Font::setSize(FontSize size)
{
    this->fontSize = size;
    this->update();
};

void Font::setType(FontType type)
{
    this->fontType = type;
    this->update();
};

void Font::update(){

};