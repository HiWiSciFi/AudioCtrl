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
    // slider value is 10 bit
    // split into high and low
    // 0ccc dddd 10dd dddd
    // c = slider index
    // d = data
    while (!Serial.available())
        ;

    while (Serial.available())
        Serial.read();

    for (uint8_t i = 0; i < ARRAY_SIZE(sliders); i++)
    {
        char buffer[2];
        memset(buffer, 0, ARRAY_SIZE(buffer));

        int value = getSliderValue(i);
        buffer[0] = ((i & 0b111) << 4) | ((value >> 6) & 0b1111);
        buffer[1] = 0b10000000 | (value & 0b111111);

        Serial.write(buffer, 2);
    }
}

// X X OUT 5V
// ...
//   X     GND