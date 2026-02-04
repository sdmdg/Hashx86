/**
 * @file        Hgui.cpp
 * @brief       Hgui Graphics Initialization
 *
 * @date        01/02/2026
 * @version     1.0.0
 */

#include <Hx86/Hgui/Hgui.h>

void init_graphics() {
    if (!desktop) {
        desktop = new Desktop();
    }
};
