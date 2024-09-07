/*
 * DRV8870.h - DRV8870 library for Wiring/Arduino - Version 1.0.0
 *
 * Original library        (1.0.0)   by Rodney Osodo.
 *
 * The moded the DRV8870 motor driver can run are listed below:
 *
 *    x_PWM1 x_PWM2    Mode
 *      0      0       Coast/Fast decay
 *      0      1       Reverse
 *      1      0       Forward
 *      1      1       Brake/slow decay
 */

#ifndef DRV8870_H
#define DRV8870_H

#define COAST 1
#define BRAKE 0
#define CLOCKWISE 1
#define COUNTERCLOCKWISE 0

#include "Arduino.h"
// #if defined(ESP32)
// #include <analogWrite.h>
// #endif
// #if defined(ESP8266)
// #include <analogWrite.h>
// #endif

class DRV8870
{
private:
    // The library version number
    int _version = 1;

    // The maximum value for an analogWrite(pin, value). This varies from board to board
    int max_speed = 255;

    // motor pin numbers. It can driver up to a maximum of 4 motors
    int _motor_pin_1;
    int _motor_pin_2;
    int _motor_pin_3;
    int _motor_pin_4;

    // Motor count
    int _motor_count;

public:
    /** Creates a DRV8870(H-bridge motor controller) control interface to drive 1 motor
     *
     * @param motor_pin_1 A PWM enabled pin, tied to the IN1 Logic input and controls state of OUT1
     * @param motor_pin_2 A PWM enabled pin, tied to the IN2 Logic input and controls state of OUT2
     *
     */
    DRV8870(int motor_pin_1, int motor_pin_2);

    /** Creates a DRV8870(H-bridge motor controller) control interface to drive 2 motors simultaneously
     *
     * @param motor_pin_1 A PWM enabled pin, tied to the IN1 Logic input and controls state of OUT1
     * @param motor_pin_2 A PWM enabled pin, tied to the IN2 Logic input and controls state of OUT2
     * @param motor_pin_3 A PWM enabled pin, tied to the IN3 Logic input and controls state of OUT3
     * @param motor_pin_4 A PWM enabled pin, tied to the IN4 Logic input and controls state of OUT4
     *
     */
    DRV8870(int motor_pin_1, int motor_pin_2, int motor_pin_3, int motor_pin_4);

    /** Set the maximum speed the motor can run
     *
     * @param max_speed The maximum speed value for an analogWrite(value);
     */
    void setMaxSpeed(int max_speed);

    /** Set the speed of the motor
     *
     * @param motor_speed The speed of the motor as a normalised value between 0 and max_speed
     */
    void setSpeed(int motor_speed, int direction);

    /** Brake the H-bridge coast or brake.
     *
     * Defaults to coast.
     * @param mode - Braking mode.COAST(default)or BRAKE.
     *
     */
    void brake(int mode);

    /** Returns the version of the library
     *
     *
     */
    int version(void);
};

#endif // DRV8870_H
