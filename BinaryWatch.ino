#include "RTClib.h"
#include "LowPower.h"

RTC_DS1307 rtc;
unsigned long start_time = 0;
bool displaying_time = false;

// Change value to correspond with activated digital I/O pins for multiplexing
uint8_t hour_leds[5] = {0b1110001, 0b1110010, 0b1110100, 0b1101001, 0b1101010};
uint8_t minute_leds[6] = {0b1011001, 0b1011010, 0b1011100, 0b0111001, 0b0111010, 0b0111100};

void setup () {
  while (!Serial);

  Serial.begin(57600);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  
  if (! rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Setup the button
  pinMode(3, INPUT);

  // Setup the output pins
  for (int i = 6; i <= 12; i++) {
    pinMode(i, OUTPUT);
  }
}

// Turn on one LED using multiplexing
void switchLEDByBits(uint8_t bits) {
  for (int i = 0; i < 7; i++) {
    digitalWrite(i + 6, bits >> i & 0x1);
  }
  delay(1);
}

// Triggered when interrupt to wake from power down state
void wakeUp() {
  displaying_time = true;
  start_time = millis();
}

void loop () {
    // Get time from RTC module
    if (displaying_time) {
      DateTime now = rtc.now();
  
      // Display hour
      for (int i = 0; i < 5; i++) {
        if (now.hour() >> i & 0x1) {
          switchLEDByBits(hour_leds[i]);
        }
      }
  
      // Display minute
      for (int i = 0; i < 6; i++) {
        if (now.minute() >> i & 0x1) {
          switchLEDByBits(minute_leds[i]);
        }
      }

      // Display time for 5 seconds
      if (millis() - start_time >= 5000) {
        displaying_time = false;
        switchLEDByBits(0);
      }
      
    } else {
      // Allow wake up pin to trigger interrupt on rising.
      attachInterrupt(digitalPinToInterrupt(3), wakeUp, RISING);
      
      // Enter power down state with ADC and BOD module disabled.
      // Wake up when wake up pin is rising.
      LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
  
      detachInterrupt(digitalPinToInterrupt(3));
    }
}
