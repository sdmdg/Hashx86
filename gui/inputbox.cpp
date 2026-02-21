/**
 * @file        inputbox.cpp
 * @brief       InputBox (part of #x86 GUI Framework)
 *
 * @date        10/01/2026
 * @version     1.0.0
 */

#define KDBG_COMPONENT "GUI:INPUTBOX"
#include <gui/inputbox.h>
#include <utils/string.h>

// --- Minimal helpers since standard C library is not available ---
static void memmove_local(char* dst, const char* src, uint32_t n) {
    if (dst < src) {
        for (uint32_t i = 0; i < n; i++) dst[i] = src[i];
    } else {
        for (uint32_t i = n; i > 0; i--) dst[i - 1] = src[i - 1];
    }
}

static int isprint_local(char c) {
    return (c >= 32 && c <= 126);  // printable ASCII
}
// ----------------------------------------------------------------

InputBox::InputBox(Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t capacity)
    : Widget(parent, x, y, w, h), capacity(capacity), length(0), cursorPos(0) {
    this->font = FontManager::activeInstance->getNewFont();
    text = new char[capacity];
    if (!text) {
        HALT("CRITICAL: Failed to allocate inputbox text buffer!\n");
    }
    text[0] = '\0';  // start empty
}

InputBox::~InputBox() {
    delete[] text;
}

void InputBox::update() {
    for (uint32_t i = 0; i < w * h; i++) cache[i] = 0;  // clear
    isDirty = true;
}

void InputBox::setText(const char* newText) {
    uint32_t newLen = strlen(newText);
    if (newLen >= capacity) newLen = capacity - 1;

    for (uint32_t i = 0; i < newLen; i++) text[i] = newText[i];

    text[newLen] = '\0';
    length = newLen;
    cursorPos = length;
    update();
}

void InputBox::setSize(FontSize size) {
    this->font->setSize(size);
    update();
}

void InputBox::setType(FontType type) {
    // Future: allow bold/italic variations
    // this->font->setType(type);
    // update();
}

void InputBox::RedrawToCache() {
    // KDBG1("Widget %d: Updating", this->ID);
    NINA::activeInstance->FillRoundedRectangle(
        cache, w, h, 0, 0, w, h, 3,
        isFocused ? INPUT_BACKGROUND_COLOR_ACTIVE : INPUT_BACKGROUND_COLOR_NORMAL);
    NINA::activeInstance->DrawRoundedRectangle(
        cache, w, h, 0, 0, w, h, 3,
        isFocused ? INPUT_BORDER_COLOR_ACTIVE : INPUT_BORDER_COLOR_NORMAL);
    NINA::activeInstance->DrawString(cache, w, h + 2, 2, 2, text, font,
                                     isFocused ? INPUT_TEXT_COLOR_ACTIVE : INPUT_TEXT_COLOR_NORMAL);

    // Draw cursor at cursorPos
    // int cursorX = font->GetTextWidth(text, cursorPos);
    // NINA::activeInstance->DrawLine(cache, w, h, 2 + cursorX, 2, 2 + cursorX, h - 2, 0xFFFFFFFF);

    isDirty = false;
}

void InputBox::Draw(GraphicsDriver* gc) {
    Widget::Draw(gc);
}

void InputBox::OnKeyDown(const char* key) {
    if (!key || !*key) return;

    // Backspace
    if (strcmp(key, (char*)"Backspace") == 0) {
        if (cursorPos > 0) {
            memmove_local(&text[cursorPos - 1], &text[cursorPos], length - cursorPos + 1);
            cursorPos--;
            length--;
            update();
        }
        return;
    }

    // Normal printable characters
    if (length < capacity - 1 && isprint_local(key[0])) {
        memmove_local(&text[cursorPos + 1], &text[cursorPos], length - cursorPos + 1);
        text[cursorPos] = key[0];
        cursorPos++;
        length++;
        text[length] = '\0';
        update();
    }
}

void InputBox::OnKeyUp(const char* key) {
    // Usually not needed for text input
    (void)key;
}
