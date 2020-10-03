#ifndef MESSAGE_H
#define MESSAGE_H
#include <QByteArray>

typedef struct keydownevent {
    int keycode;
} KeyDownEvent;

typedef struct keyupevent {
    int keycode;
} KeyUpEvent;

typedef struct mousepressevent_t{
    int x;
    int y;
} MousePressEvent;

typedef struct mousemoveevent_t{
    int x;
    int y;
} MouseMoveEvent;

typedef struct mouserightpressevent_t{
    int x;
    int y;
} MouseRightPressEvent;

typedef struct mousemiddlepressevent_t{
    int x;
    int y;
} MouseMiddlePressEvent;

typedef struct mousescrollevent_t{
    unsigned char pos;
} MouseScrollEvent;

typedef struct mousereleaseevent_t{
    int x;
    int y;
} MouseReleaseEvent;

typedef struct mouserightreleaseevent_t{
    int x;
    int y;
} MouseRightReleaseEvent;

typedef struct mousemiddlereleaseevent_t{
    int x;
    int y;
} MouseMiddleReleaseEvent;

typedef union event_t{
    KeyDownEvent keydownevent;
    KeyUpEvent keyupevent;
    MousePressEvent mousepressevent;
    MouseMoveEvent mousemoveevent;
    MouseScrollEvent mousescrollevent;
    MouseRightPressEvent mouserightpressevent;
    MouseMiddlePressEvent mousemiddlepressevent;
    MouseReleaseEvent mousereleaseevent;
    MouseRightReleaseEvent mouserightreleaseevent;
    MouseMiddleReleaseEvent mousemiddlereleaseevent;
} EventInfo;

typedef struct message_t {
    unsigned char message_type;
    EventInfo message_info;
} Message;

#endif // MESSAGE_H
