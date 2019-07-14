/*
***************************************************************************  
**  Program  : IR_receiver
**
**  This program is mostly a copy of IRrecvDump by David Conran and others
** 
**  The main purpose of this program is to test the IR-receiver circuit
**
*/
#define _FW_VERSION "v0.1.0 (2019-07-14)"
/*
 * =====================================================================
 * IRremoteESP8266: IRrecvDump - dump details of IR codes with IRrecv
 * An IR detector/demodulator must be connected to the input kRecvPin.
 *
 * Copyright 2009 Ken Shirriff, http://arcfn.com
 * Copyright 2017 David Conran
 *
 * Example circuit diagram:
 *  https://github.com/markszabo/IRremoteESP8266/wiki#ir-receiving
 *
 * Changes:
 *   Version 0.3 November, 2017
 *     - Support for A/C decoding for some protcols.
 *   Version 0.2 April, 2017
 *     - Decode from a copy of the data so we can start capturing faster thus
 *       reduce the likelihood of miscaptures.
 * Based on Ken Shirriff's IrsendDemo Version 0.1 July, 2009,
 */
#include <FS.h>

#include <IRremoteESP8266.h>  // https://github.com/markszabo/IRremoteESP8266 [v2.6.2 (20190616)]
#include <IRrecv.h>
#include <IRutils.h>

// ==================== start of TUNEABLE PARAMETERS ====================
// An IR detector/demodulator is connected to GPIO02
const uint16_t kRecvPin = 12;

// The Serial connection baud rate.
// i.e. Status message will be sent to the PC at this baud rate.
// Try to avoid slow speeds like 9600, as you will miss messages and
// cause other problems. 115200 (or faster) is recommended.
// NOTE: Make sure you set your Serial Monitor to the same speed.
const uint32_t kBaudRate = 115200;

// As this program is a special purpose capture/decoder, let us use a larger
// than normal buffer so we can handle Air Conditioner remote codes.
const uint16_t kCaptureBufferSize = 1024;

// kTimeout is the Nr. of milli-Seconds of no-more-data before we consider a
// message ended.
// This parameter is an interesting trade-off. The longer the timeout, the more
// complex a message it can capture. e.g. Some device protocols will send
// multiple message packets in quick succession, like Air Conditioner remotes.
// Air Coniditioner protocols often have a considerable gap (20-40+ms) between
// packets.
// The downside of a large timeout value is a lot of less complex protocols
// send multiple messages when the remote's button is held down. The gap between
// them is often also around 20+ms. This can result in the raw data be 2-3+
// times larger than needed as it has captured 2-3+ messages in a single
// capture. Setting a low timeout value can resolve this.
// So, choosing the best kTimeout value for your use particular case is
// quite nuanced. Good luck and happy hunting.
// NOTE: Don't exceed kMaxTimeoutMs. Typically 130ms.
#if DECODE_AC
// Some A/C units have gaps in their protocols of ~40ms. e.g. Kelvinator
// A value this large may swallow repeats of some protocols
const uint8_t kTimeout = 50;
#else   // DECODE_AC
// Suits most messages, while not swallowing many repeats.
const uint8_t kTimeout = 15;
#endif  // DECODE_AC
// Alternatives:
// const uint8_t kTimeout = 90;
// Suits messages with big gaps like XMP-1 & some aircon units, but can
// accidentally swallow repeated messages in the rawData[] output.
//
// const uint8_t kTimeout = kMaxTimeoutMs;
// This will set it to our currently allowed maximum.
// Values this high are problematic because it is roughly the typical boundary
// where most messages repeat.
// e.g. It will stop decoding a message and start sending it to serial at
//      precisely the time when the next message is likely to be transmitted,
//      and may miss it.

// Set the smallest sized "UNKNOWN" message packets we actually care about.
// This value helps reduce the false-positive detection rate of IR background
// noise as real messages. The chances of background IR noise getting detected
// as a message increases with the length of the kTimeout value. (See above)
// The downside of setting this message too large is you can miss some valid
// short messages for protocols that this library doesn't yet decode.
//
// Set higher if you get lots of random short UNKNOWN messages when nothing
// should be sending a message.
// Set lower if you are sure your setup is working, but it doesn't see messages
// from your device. (e.g. Other IR remotes work.)
// NOTE: Set this value very high to effectively turn off UNKNOWN detection.
const uint16_t kMinUnknownSize = 12;
// ==================== end of TUNEABLE PARAMETERS ====================

#define RED_LEDPIN    5
#define GRN_LEDPIN    14

char      inChar;       // Console input

// Use turn on the save buffer feature for more complete capture coverage.
IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);

decode_results results;  // Somewhere to store the results

// Display the human readable state of an A/C message if we can.
void dumpACInfo(decode_results *results) {
  String description = "";
  // If we got a human-readable description of the message, display it.
  if (description != "")  Serial.println("Mesg Desc.: " + description);
  
} // dumpACInfo()


//---------------------------------------------------------------------------
void setup() {
//Serial.begin(BAUD_RATE, SERIAL_8N1, SERIAL_TX_ONLY);
  Serial.begin(115200);
  while (!Serial)  // Wait for the serial connection to be establised.
    delay(50);
  Serial.println("\n");

  pinMode(RED_LEDPIN, OUTPUT);
  pinMode(GRN_LEDPIN, OUTPUT);

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
    Serial.println("SPIFFS Mount failed");           // Serious problem with SPIFFS 
  } else { 
    Serial.println("SPIFFS Mount successful");
  }

#if DECODE_HASH
  // Ignore messages with less than minimum on or off pulses.
  irrecv.setUnknownThreshold(kMinUnknownSize);
#endif  // DECODE_HASH
  irrecv.enableIRIn();  // Start the receiver

//pinMode(kRecvPin, INPUT_PULLUP);
  Serial.println();
  Serial.print("IR_receiver is now running and waiting for IR input on Pin ");
  Serial.println(kRecvPin);

} // setup()


// The repeating section of the code
//
void loop() {
  handleKeyInput(); // menu

  digitalWrite(GRN_LEDPIN, HIGH);
  
  // Check if the IR code has been received.
  if (irrecv.decode(&results)) {
    // Display a crude timestamp.
    uint32_t now = millis();
    Serial.printf("Timestamp : %06u.%03u\n", now / 1000, now % 1000);
    if (results.overflow)
      Serial.printf("WARNING: IR code is too big for buffer (>= %d). "
                    "This result shouldn't be trusted until this is resolved. "
                    "Edit & increase kCaptureBufferSize.\n",
                    kCaptureBufferSize);
    // Display the basic output of what we found.
    Serial.print(resultToHumanReadableBasic(&results));
    dumpACInfo(&results);  // Display any extra A/C info if we have it.
    yield();  // Feed the WDT as the text output can take a while to print.

    // Display the library version the message was captured with.
    Serial.print("Library   : v");
    Serial.println(_IRREMOTEESP8266_VERSION_);
    Serial.println();

    // Output RAW timing info of the result.
    Serial.println(resultToTimingInfo(&results));
    yield();  // Feed the WDT (again)

    // Output the results as source code
    Serial.println(resultToSourceCode(&results));
    Serial.println("");  // Blank line between entries
    yield();  // Feed the WDT (again)
  }
}
