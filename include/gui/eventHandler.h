#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <utils/linkedList.h>

typedef enum {
    ON_CLICK = 0x0,
    ON_KEYPREASS = 0x1,
    ON_WINDOW_CLOSE = 0x2,
} EVENT_TYPE;

struct Event {
    uint32_t widgetID;
    EVENT_TYPE eventType;
    uint32_t param1;
    uint32_t param2;
};

#endif // EVENT_HANDLER_H