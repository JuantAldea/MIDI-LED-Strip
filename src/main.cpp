#include <Arduino.h>
#include <MIDIUSB.h>
#include <FastLED.h>

#define LED_PIN     5
#define NUM_LEDS    144
#define BRIGHTNESS  8
#define LED_TYPE    WS2813
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

bool sustain_on = false;

void note(byte channel, byte pitch, byte velocity) {
    Serial.print("note=");
    Serial.print(pitch);
    Serial.print(", channel=");
    Serial.print(channel);
    Serial.print(", velocity=");
    Serial.println(velocity);
}

void control(byte channel, byte control, byte value) {
    Serial.print("control=");
    Serial.print(control, HEX);
    Serial.print(", value=");
    Serial.print(value, HEX);
    Serial.print(", channel=");
    Serial.println(channel, HEX);

    switch (control) {
    case 0x7B:
        FastLED.clear(true);
        break;
    case 0x40:
        sustain_on = value >= 64;
        break;
    }
}

void print_unhandled(const midiEventPacket_t &rx)
{
    Serial.print("MIDI message: ");
    Serial.print(rx.header, HEX);
    Serial.print("-");
    Serial.print(rx.byte1, HEX);
    Serial.print("-");
    Serial.print(rx.byte2, HEX);
    Serial.print("-");
    Serial.println(rx.byte3, HEX);
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(9600);
    delay(3000);
    Serial.println("Starting...");
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, 144).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);

    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = i % 2 == 0 ? CRGB::White: CRGB::Black;
    }

    FastLED.show();
    delay(500);

    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = i % 2 != 0 ? CRGB::White : CRGB::Black;
    }

    FastLED.show();

    delay(500);

    FastLED.clear(true);

    FastLED.show();
    delay(500);
    Serial.println("Ready.");
}


void loop() {
    midiEventPacket_t rx;
    while(rx = MidiUSB.read(), rx.header) {
        switch (rx.header) {
            case 0x9:
            note(rx.byte1 & 0xF, rx.byte2, rx.byte3);

            if ((rx.byte2 - 21) * 2 >= NUM_LEDS){
                continue;
            }

            leds[(rx.byte2 - 21) * 2] = rx.byte3 ? CRGB::White : CRGB::Black;
            break;

            case 0xB:
            control(rx.byte1 & 0xF, rx.byte2, rx.byte3);
            break;

            default:
            print_unhandled(rx);
            break;
        }

        FastLED.show();
        MidiUSB.flush();
    }
}