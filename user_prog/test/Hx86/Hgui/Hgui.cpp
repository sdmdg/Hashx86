#include <Hx86/Hgui/Hgui.h>

void init_graphics()
{
    if (!desktop){
        desktop = new Desktop();
    }
};