#include "Application.hpp"

#include <Servo.h>

Application application;
Servo servoMotor;

decltype(millis()) lastMillis = 0;
int count = 0;
bool state = LOW;

void setup()
{
    Serial.begin(9600);
    servoMotor.attach(2);
    pinMode(LED_BUILTIN, OUTPUT);
    application.Init();
}

void loop()
{
    if (lastMillis < millis())
    {
        digitalWrite(LED_BUILTIN, state);
        state = 1 - state;
        servoMotor.write(30); // count * (180.0F / 5.0F)
        Serial.printf("t=%d, ", lastMillis);
        lastMillis += 1000;
        if (5 <= count)
        {
            count = 0;
            Serial.print("\n");
        }
        else
        {
            count++;
        }
    }
    application.Update();
}
