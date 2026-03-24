#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <gui/config/config.h>
#include <gui/widget.h>
#include <types.h>

class ProgressBar : public Widget {
private:
    float progress;  // 0.0 to 1.0
    uint32_t barColor;
    uint32_t backgroundColor;

public:
    ProgressBar(Widget* parent, int32_t x, int32_t y, int32_t w, int32_t h,
                float initialProgress = 0.0f);
    ~ProgressBar();

    // Progress management
    void SetProgress(float progress);  // 0.0 to 1.0
    float GetProgress() const;
    void SetPercentage(int32_t percentage);  // 0 to 100

    // Appearance (optional override)
    void SetBarColor(uint32_t color);
    void SetBackgroundColor(uint32_t color);

    // Redraw
    void RedrawToCache() override;
    void update();
};

#endif  // PROGRESSBAR_H
