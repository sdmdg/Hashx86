/**
 * @file        progressbar.cpp
 * @brief       ProgressBar Widget (part of #x86 GUI Framework)
 *
 * @date        18/03/2026
 * @version     1.0.0
 */

#define KDBG_COMPONENT "GUI:PROGRESSBAR"
#include <gui/config/config.h>
#include <gui/progressbar.h>

ProgressBar::ProgressBar(Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h,
                         float initialProgress)
    : Widget(parent, x, y, w, h), progress(0.0f), barColor(0), backgroundColor(0) {
    this->isFocussable = false;
    this->progress = initialProgress;
    if (this->progress < 0.0f) this->progress = 0.0f;
    if (this->progress > 1.0f) this->progress = 1.0f;

    // Allocate cache
    if (cache) {
        delete[] cache;
        cache = nullptr;
    }

    if (w > 0 && h > 0) {
        cache = new uint32_t[w * h]();
        if (!cache) {
            HALT("CRITICAL: Failed to allocate progressbar cache!\n");
        }
    }

    MarkDirty();
}

ProgressBar::~ProgressBar() {
    // cache deleted by ~Widget
}

void ProgressBar::SetProgress(float progress) {
    this->progress = progress;
    if (this->progress < 0.0f) this->progress = 0.0f;
    if (this->progress > 1.0f) this->progress = 1.0f;
    MarkDirty();
}

float ProgressBar::GetProgress() const {
    return this->progress;
}

void ProgressBar::SetPercentage(int32_t percentage) {
    if (percentage < 0) percentage = 0;
    if (percentage > 100) percentage = 100;
    this->progress = static_cast<float>(percentage) / 100.0f;
    MarkDirty();
}

void ProgressBar::SetBarColor(uint32_t color) {
    this->barColor = color;
    MarkDirty();
}

void ProgressBar::SetBackgroundColor(uint32_t color) {
    this->backgroundColor = color;
    MarkDirty();
}

void ProgressBar::RedrawToCache() {
    // Clear cache
    memset(cache, 0, sizeof(uint32_t) * w * h);

    // Use config colors by default, or custom colors if set
    uint32_t bgColor = (backgroundColor != 0) ? backgroundColor : PROGRESSBAR_BACKGROUND_COLOR;
    uint32_t fillColor = (barColor != 0) ? barColor : PROGRESSBAR_BAR_COLOR;

    // Draw background rectangle
    NINA::activeInstance->FillRoundedRectangle(cache, w, h, 0, 0, w, h, 3, bgColor);

    // Calculate bar dimensions
    int32_t barWidth = static_cast<int32_t>(w * progress);
    if (barWidth > w) barWidth = w;

    // Draw progress bar (filled rectangle on top of background) only when progress > 0.
    if (barWidth > 0) {
        NINA::activeInstance->FillRoundedRectangle(cache, w, h, 0, 0, barWidth, h, 3, fillColor);
    }

    isDirty = false;
}

void ProgressBar::update() {
    MarkDirty();
}
