#include <IRremote.h>
#include "Tlc5940.h"
#include "tlc_fades.h"

#define MAX_SPEED 3
#define MAX_MODE 2

decode_results results;
IRrecv ir(2);

TLC_CHANNEL_TYPE channel;


uint8_t mode = 1;

// max 1
float brightness = 1;

// SPEED
uint8_t speed = 1;
uint16_t fade_duration;
float whatever_freq;


/// WHATEVER fading variables
int value = 0;
float inter;
float lag = 0.4;
float a = 7;

void updateSpeeds() {
  if (speed == 1) {
    fade_duration = 3000;
    whatever_freq = 1;
  } else if (speed == 2) {
    fade_duration = 1500;
    whatever_freq = 4;
  } else if (speed == 3) {
    fade_duration = 500;
    whatever_freq = 8;
  }
}

void setup() {
  // put your setup code here, to run once:
  

  Serial.begin(9600);
  Serial.println("hello");

  Tlc.init(); // start the driver
  //ir.enableIRIn(); // Start the receiver

  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);

  updateSpeeds();
}

void loop() {
  if (mode == 0) {
    // STATIC MODE (all on)
    Tlc.setAll((int) (4095*brightness));
    Tlc.update();
  } else if (mode == 1) {
    // WHATEVER MODE (nice fade)
    for (int channel = 0; channel < NUM_TLCS * 16; channel += 1) {
      inter = (1+sin(whatever_freq* micros()/1000000.0 + channel*lag))/2.0;
      inter = brightness*(exp(a*inter)-1)/(exp(a)-1);
    
      value = 4095*inter; 
        //Serial.println(value);
        if (channel == 13) {
          Tlc.set(channel,4095-value);
        } else {
          Tlc.set(channel,value);
        }
      }
      Tlc.update();
  } else if (mode == 2) {
    // FADE MODE (default fade
      if (tlc_fadeBufferSize < TLC_FADE_BUFFER_LENGTH - 2) {
      if (!tlc_isFading(channel)) {
        int maxValue = (int) 4095*brightness;
        uint32_t startMillis = millis() + 50;
        uint32_t endMillis = startMillis + fade_duration;
        tlc_addFade(channel, 0, maxValue, startMillis, endMillis);
        tlc_addFade(channel, maxValue, 0, endMillis, endMillis + fade_duration);
      }
      if (channel++ == NUM_TLCS * 16) {
        channel = 0;
      }
    }
    tlc_updateFades();
  }
  
  if (digitalRead(6) == LOW || digitalRead(7) == LOW) {
    delay(20);
    tlc_removeFades(channel);
    if (digitalRead(6) == LOW && digitalRead(7) == LOW) {
      brightness = 0;
      Tlc.setAll(0);
      Tlc.update();
    } else if (digitalRead(6) == LOW) {
      speed = (speed + 1);
      if (speed > MAX_SPEED || mode == 0) {
        // static mode doesn't care about speed
        mode = (mode + 1) % (MAX_MODE+1);
        speed = 1;
      }
      updateSpeeds();
    } else if (digitalRead(7) == LOW) {      
      if (brightness > 0.01) brightness = (brightness/3.0);
      else brightness = 1;

      Serial.println(brightness);
      
    }

    do {
      delay(300); // debouncing
    } while (digitalRead(6) == LOW || digitalRead(7) == LOW);
    Serial.println("released");
  }

  //delay(100);
}
