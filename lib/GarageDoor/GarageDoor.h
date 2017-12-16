#ifndef GarageDoor_h
#define GarageDoor_h

#include "Arduino.h"

class GarageDoor
{
    public:
        GarageDoor(int open_button_pin, int close_button_pin, int sensor_pin);
        void open();
        void close();
        bool is_closed();
        void update();
        void on_change(void (*callback)());
    private:
        // may need some kind of timestamp to see how long it has been
        // assume open if past some point in time
        int _open_button_pin;
        int _close_button_pin;
        int _sensor_pin;
        bool _sensor_status;
        void (*callback)();
        void handle_change();
};

#endif