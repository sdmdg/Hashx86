/**
 * @file        segoeui.cpp
 * @brief       Segoeui font.data (part of #x86 GUI Framework)
 * 
 * @date        12/02/2025
 * @version     1.0.0-beta
 */

#include <gui/fonts/segoeui.h>


SegoeUI::SegoeUI()
{
    this->fontSize = MEDIUM;
    this->fontType = REGULAR;
    this->update();
};

SegoeUI::~SegoeUI() {

};

void SegoeUI::update()
{   
    switch (this->fontSize)
    {
    case XLARGE:
        switch (this->fontType)
        {
        case REGULAR:
            this->font_36x36 = (uint32_t(*)[684])font_24_36x36_segoeui_regular;
            this->chrAdvanceX = 2;
            break;
        /* case BOLT:
            this->font_36x36 = (uint32_t(*)[684])font_24_36x36_segoeui_bolt;
            this->chrAdvanceX = 4;
            break;
        case ITALIC:
            this->font_36x36 = (uint32_t(*)[684])font_24_36x36_segoeui_italic;
            this->chrAdvanceX = 3;
            break; */
        default:
            break;
        }
        this->font_36x36_config = (uint8_t(*)[4])font_24_36x36_segoeui_regular_config;
        this->chrAdvanceY = -9;
        this->chrHeight = 20;
        break;

    case LARGE:
        switch (this->fontType)
        {
        case REGULAR:
            this->font_36x36 = (uint32_t(*)[684])font_16_36x36_segoeui_regular;
            this->chrAdvanceX = 2;
            break;
        /* case BOLT:
            // this->font_36x36 = (uint32_t (*)[684])font_16_36x36_segoeui_bolt;
            this->chrAdvanceX = 4;
            break;
        case ITALIC:
            // this->font_36x36 = (uint32_t (*)[684])font_16_36x36_segoeui_italic;
            this->chrAdvanceX = 3;
            break; */
        default:
            break;
        }
        this->font_36x36_config = (uint8_t(*)[4])font_16_36x36_segoeui_regular_config;
        this->chrAdvanceY = -6;
        this->chrHeight = 13;
        break;

    case MEDIUM:
        switch (this->fontType)
        {
        case REGULAR:
            this->font_36x36 = (uint32_t(*)[684])font_12_36x36_segoeui_regular;
            this->chrAdvanceX = 2;
            break;
        /* case BOLT:
            this->font_36x36 = (uint32_t (*)[684])font_12_36x36_segoeui_bolt;
            this->chrAdvanceX = 4;
            break;
        case ITALIC:
            this->font_36x36 = (uint32_t (*)[684])font_12_36x36_segoeui_italic;
            this->chrAdvanceX = 3;
            break; */
        default:
            break;
        }
        this->font_36x36_config = (uint8_t(*)[4])font_12_36x36_segoeui_regular_config;
        this->chrAdvanceY = -14;//.5
        this->chrHeight = 10;
        break;

    case SMALL:
        switch (this->fontType)
        {
        case REGULAR:
            this->font_36x36 = (uint32_t(*)[684])font_8_36x36_segoeui_regular;
            this->chrAdvanceX = 2;
            break;
        /* case BOLT:
            //this->font_36x36 = (uint32_t(*)[684])font_8_36x36_segoeui_bolt;
            this->chrAdvanceX = 4;
            break;
        case ITALIC:
            //this->font_36x36 = (uint32_t(*)[684])font_8_36x36_segoeui_italic;
            this->chrAdvanceX = 3;
            break; */
        default:
            break;
        }
        this->font_36x36_config = (uint8_t(*)[4])font_8_36x36_segoeui_regular_config;
        this->chrAdvanceY = -3;
        this->chrHeight = 7;
        break;
    default:
        break;
    }
};
