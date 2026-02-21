/**
 * @file        label.cpp
 * @brief       Label Component (part of #x86 GUI Framework)
 *
 * @date        01/02/2026
 * @version     1.0.0
 */

#define KDBG_COMPONENT "GUI:LABEL"
#include <gui/label.h>

Label::Label(Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h, const char* text)
    : Widget(parent, x, y, w, h) {
    this->font = FontManager::activeInstance->getNewFont();
    this->text = new char[strlen(text) + 1];
    if (!this->text) {
        HALT("CRITICAL: Failed to allocate label text!\n");
    }
    strcpy(this->text, text);
}

Label::~Label() {
    delete[] text;
    // cache deleted by ~Widget
}

void Label::setText(const char* newText) {
    if (this->text && strcmp(this->text, newText) == 0) return;

    if (this->text) delete[] this->text;
    this->text = new char[strlen(newText) + 1];
    if (!this->text) {
        HALT("CRITICAL: Failed to allocate label text!\n");
    }
    strcpy(this->text, newText);
    MarkDirty();
}

void Label::setSize(FontSize size) {
    this->font->setSize(size);
    MarkDirty();
}

void Label::RedrawToCache() {
    // Clear background to transparent (0)
    memset(cache, 0, sizeof(uint32_t) * w * h);

    NINA::activeInstance->DrawString(cache, w, h, 2, 2, text, font, LABEL_TEXT_COLOR_NORMAL);
    isDirty = false;
}

void Label::update() {
    MarkDirty();
}
