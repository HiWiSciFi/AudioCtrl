#include <Arduino.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*a))

uint8_t sliders[6] = {A0, A1, A2, A3, A6, A7};

void setupSlider(int index)
{
    pinMode(sliders[index], INPUT);
}

int getSliderValue(int index)
{
    return 1023 - analogRead(sliders[index]);
}

void setup()
{
    for (uint8_t i = 0; i < ARRAY_SIZE(sliders); i++)
        setupSlider(i);
    Serial.begin(9600);
}

void loop()
{
    for (uint8_t i = 0; i < ARRAY_SIZE(sliders); i++)
    {
        if (i != 0)
            Serial.print("\t");
        Serial.print(getSliderValue(i));
    }
    Serial.println();
}

// X X OUT 5V
// ...
//   X     GND