#include <DRV8870.h>

// Pin 10 ENA is tied to HIGH
// Pin 8 is IN1
// Pin 9 is IN2
DRV8870 mymotor(8, 9);

void setup()
{
    mymotor.setMaxSpeed(255);
    pinMode(10, OUTPUT);
    digitalWrite(10, HIGH);
}

void loop()
{
    mymotor.setSpeed(255, CLOCKWISE);
    delay(50000);
}