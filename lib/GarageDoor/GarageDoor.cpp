#include "Arduino.h"
#include "GarageDoor.h"

GarageDoor::GarageDoor(int open_button_pin, int close_button_pin, int sensor_pin)
{
    _open_button_pin = open_button_pin;
    _close_button_pin = close_button_pin;
    _sensor_pin = sensor_pin;

    pinMode(_open_button_pin, OUTPUT);
    pinMode(_close_button_pin, OUTPUT);
    pinMode(_sensor_pin, INPUT);

    digitalWrite(_open_button_pin, LOW);
    digitalWrite(_close_button_pin, LOW);
}

void GarageDoor::open()
{
    digitalWrite(_open_button_pin, HIGH);
    delay(100);
    digitalWrite(_open_button_pin, LOW);
}

void GarageDoor::close()
{
    digitalWrite(_close_button_pin, HIGH);
    delay(100);
    digitalWrite(_close_button_pin, LOW);
}

bool GarageDoor::is_closed()
{
    return _sensor_status == HIGH;
}

void GarageDoor::update()
{
    int previous_status = _sensor_status;
    _sensor_status = digitalRead(_sensor_pin);

    if(previous_status != _sensor_status)
    {
        handle_change();
    }
}

void GarageDoor::handle_change()
{
    callback();
    // call custom on_change function here
}

void GarageDoor::on_change(void (*callback)())
{
    this->callback = callback;
}