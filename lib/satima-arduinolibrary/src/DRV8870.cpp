#include "DRV8870.h"

DRV8870::DRV8870(int motor_pin_1, int motor_pin_2)
{
    this->_motor_count = 1;
    this->_motor_pin_1 = motor_pin_1;
    this->_motor_pin_2 = motor_pin_2;
    pinMode(this->_motor_pin_1, OUTPUT);
    pinMode(this->_motor_pin_2, OUTPUT);
}
DRV8870::DRV8870(int motor_pin_1, int motor_pin_2, int motor_pin_3, int motor_pin_4)
{
    this->_motor_count = 2;
    this->_motor_pin_1 = motor_pin_1;
    this->_motor_pin_2 = motor_pin_2;
    this->_motor_pin_3 = motor_pin_3;
    this->_motor_pin_4 = motor_pin_4;
    pinMode(this->_motor_pin_1, OUTPUT);
    pinMode(this->_motor_pin_2, OUTPUT);
    pinMode(this->_motor_pin_3, OUTPUT);
    pinMode(this->_motor_pin_4, OUTPUT);
}
void DRV8870::setMaxSpeed(int max_speed)
{
    this->max_speed = max_speed;
}
void DRV8870::setSpeed(int motor_speed, int direction)
{
    switch (this->_motor_count)
    {
    case 1:
        switch (direction)
        {
        case CLOCKWISE:
            analogWrite(this->_motor_pin_1, motor_speed);
            analogWrite(this->_motor_pin_2, 0);
            break;
        case COUNTERCLOCKWISE:
            analogWrite(this->_motor_pin_1, 0);
            analogWrite(this->_motor_pin_2, motor_speed);
            break;
        default:
            break;
        }
        break;
    case 2:
        switch (direction)
        {
        case CLOCKWISE:
            analogWrite(this->_motor_pin_1, motor_speed);
            analogWrite(this->_motor_pin_2, 0);
            analogWrite(this->_motor_pin_3, motor_speed);
            analogWrite(this->_motor_pin_4, 0);
            break;
        case COUNTERCLOCKWISE:
            analogWrite(this->_motor_pin_1, 0);
            analogWrite(this->_motor_pin_2, motor_speed);
            analogWrite(this->_motor_pin_3, 0);
            analogWrite(this->_motor_pin_4, motor_speed);
            break;
        default:
            break;
        }
        break;

    default:
        break;
    }
}
void DRV8870::brake(int mode)
{
    if (mode == COAST)
    {
        switch (this->_motor_count)
        {
        case 1:
            analogWrite(this->_motor_pin_1, LOW);
            analogWrite(this->_motor_pin_2, LOW);
            break;
        case 2:
            digitalWrite(this->_motor_pin_1, LOW);
            digitalWrite(this->_motor_pin_2, LOW);
            digitalWrite(this->_motor_pin_3, LOW);
            digitalWrite(this->_motor_pin_4, LOW);
            break;
        default:
            break;
        }
    }
    else if (mode == BRAKE)
    {
        switch (this->_motor_count)
        {
        case 1:
            analogWrite(this->_motor_pin_1, HIGH);
            analogWrite(this->_motor_pin_2, HIGH);
            break;
        case 2:
            digitalWrite(this->_motor_pin_1, HIGH);
            digitalWrite(this->_motor_pin_2, HIGH);
            digitalWrite(this->_motor_pin_3, HIGH);
            digitalWrite(this->_motor_pin_4, HIGH);
            break;
        default:
            break;
        }
    }
}
int DRV8870::version(void)
{
    return _version;
}
