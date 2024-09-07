#include <DRV8870.h>

// Pin 10 ENA is tied to HIGH
// Pin 8 is IN1
// Pin 9 is IN2
#define EN_PIN 10
DRV8870 mymotor(8, 9);

void setup()
{
    Serial.begin(9600);
    pinMode(EN_PIN, OUTPUT);
    digitalWrite(EN_PIN, HIGH);
    mymotor.setMaxSpeed(255);
}

void loop()
{
    mymotor.setSpeed(255, CLOCKWISE);
    Serial.println("clockwise");
    delay(5000);
    mymotor.brake(COAST);
    Serial.println("coast");
    mymotor.setSpeed(255, COUNTERCLOCKWISE);
    Serial.println("counterclockwise");
    delay(5000);
    mymotor.brake(BRAKE);
}
