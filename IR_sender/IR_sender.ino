/*
***************************************************************************  
**  Program  : IR_sender
**
**  Based on Ken Shirriff's IrsendDemo Version 0.1 July, 2009,
**  Copyright 2009 Ken Shirriff, http://arcfn.com
**
**  The main purpose of this program is to test the IR-sender circuit
**
*/
#define _FW_VERSION "v0.1.0 (2019-07-14)"
/*
 * =====================================================================
 * An IR LED circuit *MUST* be connected to the ESP8266 on a pin
 * as specified by kIrLed below.
 *
 * Common mistakes & tips:
 *   * Don't just connect the IR LED directly to the pin, it won't
 *     have enough current to drive the IR LED effectively.
 *   * Make sure you have the IR LED polarity correct.
 *     See: https://learn.sparkfun.com/tutorials/polarity/diode-and-led-polarity
 *   * Typical digital camera/phones can be used to see if the IR LED is flashed.
 *     Replace the IR LED with a normal LED if you don't have a digital camera
 *     when debugging.
 */

#include <FS.h>
#include <IRsend.h>

#define IR_LEDPIN    4
#define RED_LEDPIN   5
#define GRN_LEDPIN  14

IRsend  irsend(IR_LEDPIN);  // Set the GPIO to be used to sending the message.

char      inChar;
uint16_t  rawData[500];  // placeholder for read data
int16_t   noPulses;
uint32_t  blinkLed;

void setup() {
  Serial.begin(115200);
  pinMode(IR_LEDPIN,  OUTPUT);
  pinMode(RED_LEDPIN, OUTPUT);
  pinMode(GRN_LEDPIN, OUTPUT);
  Serial.print("\n\nIR_sender is now running and sending IR on pin ");
  Serial.println(IR_LEDPIN);

  for (int i=0; i < 20; i++) {
    digitalWrite(RED_LEDPIN, !digitalRead(RED_LEDPIN));
    digitalWrite(GRN_LEDPIN, !digitalRead(RED_LEDPIN));
    if (digitalRead(RED_LEDPIN))  Serial.print("H");
    else                          Serial.print("L");
    delay(250);
  }
  digitalWrite(RED_LEDPIN, LOW);
  digitalWrite(GRN_LEDPIN, LOW);
  Serial.println();
  
  if (!SPIFFS.begin()) {
    Serial.println("\nSPIFFS Mount failed");           // Serious problem with SPIFFS 
  } else { 
    Serial.println("\nSPIFFS Mount successful");
  }

  blinkLed = millis() + 10000;

}

void loop() {
  handleKeyInput(); // menu

  if (noPulses > 0) {
    digitalWrite(RED_LEDPIN, HIGH);
    Serial.printf("Sending [%d] pulses .. ", noPulses);
    for (int s=1; s < 6; s++) {
      Serial.printf("%d ", s);
      irsend.sendRaw(rawData, noPulses, 38);  // Send a raw data capture at 38kHz.
      delay(100);
    }
    digitalWrite(RED_LEDPIN, LOW);
    Serial.println();
    noPulses = -1;
    blinkLed = millis() + 10000;
  }
  if (millis() > blinkLed) {
    blinkLed = millis() + 10000;
    digitalWrite(GRN_LEDPIN, HIGH);
    delay(100);
    digitalWrite(GRN_LEDPIN, LOW);
  }
}
