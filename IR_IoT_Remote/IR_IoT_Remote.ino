/*
***************************************************************************  
**  Program  : IR_IoT_Remote
*/
#define _FW_VERSION "v0.2.2 WS (27-08-2019)"
/* 
**
**  Copyright (c) 2019 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      

    Arduino-IDE settings for ESP-12E:

    - Board: "NodeMCU 1.0 (ESP-12E Module)"
    - Flash mode: "DIO" / "DOUT"
    - Flash size: "4M (2M SPIFFS)"
    - CPU Frequency: "80 MHz"
    - Debug port: "Disabled"
    - Debug Level: "None"
    - IwIP Variant: "v2 Lower Memory"
    - VTables: "Flash"
    - Reset Method: "nodemcu" | "none"
    - CPU Frequency: "80 MHz"
    - Buildin Led: "1"
    - Upload Speed: "115200"
    - Erase Flash: "Only Sketch"
*/


#include <IRremoteESP8266.h>  // https://github.com/markszabo/IRremoteESP8266 [v2.6.2 (20190616)]
#include <IRrecv.h>
#include <IRsend.h>
#include <IRutils.h>

// ==================== start of TUNEABLE PARAMETERS ====================
// An IR detector/demodulator is connected to GPIO12
const uint16_t kRecvPin = 12;

// The Serial connection baud rate.
const uint32_t kBaudRate = 115200;

// As this program is a special purpose capture/decoder, let us use a larger
// than normal buffer so we can handle Air Conditioner remote codes.
const uint16_t kCaptureBufferSize = 1024;

// kTimeout is the Nr. of milli-Seconds of no-more-data before we consider a
// message ended.
// NOTE: Don't exceed kMaxTimeoutMs. Typically 130ms.
#if DECODE_AC
// Some A/C units have gaps in their protocols of ~40ms. e.g. Kelvinator
// A value this large may swallow repeats of some protocols
const uint8_t kTimeout = 50;
#else   // DECODE_AC
// Suits most messages, while not swallowing many repeats.
const uint8_t kTimeout = 15;
#endif  // DECODE_AC
// NOTE: Set this value very high to effectively turn off UNKNOWN detection.
const uint16_t kMinUnknownSize = 12;
// ==================== end of TUNEABLE PARAMETERS ====================

#include <OneWire.h>
#include <DallasTemperature.h>
#include <TimeLib.h>  // https://github.com/PaulStoffregen/Time

#include "Debug.h"
#define _HOSTNAME     "IRremote"
#include "networkStuff.h"

#define ONE_WIRE_PIN 13                 // GPIO02 - GPIO pin waar DS18B20 op aangesloten is
#define IR_LEDPIN     4
#define RED_LEDPIN    5
//#define kRecvPin   12
#define GRN_LEDPIN   14
#define _SETTINGS_FILE  "/settings.ini"
#define _MAX_BUTTONS  20

IRsend  irsend(IR_LEDPIN);  // Set the GPIO to be used to sending the message.
// Use turn on the save buffer feature for more complete capture coverage.
IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);

decode_results results;  // Somewhere to store the results

typedef struct {
    uint8_t Sequence;
    char    buttonLabel[30];
    char    pulseFile[32];
} dataStruct;

dataStruct  Buttons[_MAX_BUTTONS + 1];
uint16_t    maxButtons = 0;

static char *flashMode[]    { "QIO", "QOUT", "DIO", "DOUT", "UnKnown" };
enum    { TAB_CONTROL, TAB_LEARN, TAB_EDIT, TAB_UNKNOWN };

float     tn0, inTemp0 = 0.0, lastTemp, DS18B20Temp;
bool      hasDS18B20sensor;
uint32_t  nextSecond, nextStateSend, nextDS18B20PollTime, waitForPulse;
uint64_t  upTimeSeconds;
String    hostnameMAC, jsonString, lastResetReason;
char      inChar;               // Console input
char      cMsg[100];
uint16_t  rawData[500];         // placeholder for read data
uint16_t  noPulses;
char      wsSend[100];          // String to send via WebSocket
bool      newTab  = true;
int8_t    clientActiveTab;
float     clientInTemp0;

const int         timeZone = 1;       // Central European (Winter) Time
unsigned int      localPort = 8888;   // local port to listen for UDP packets

time_t            prevDisplay = 0; // when the digital clock was displayed
                  // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire           oneWire(ONE_WIRE_PIN);
                  // Pass our oneWire reference to Dallas Temperature. 
DallasTemperature DS18B20(&oneWire);
// arrays to hold device address
DeviceAddress     DS18B20_0;
DeviceAddress     DS18B20addr[8];


/***************************************************************************/
//===============START OF PROTOTYPES=========================================
//======= The Arduino IDE creates prototypes of all =========================
//======= functions so this realy is not necessary  =========================
//float   getInsideTemp(uint8_t); 
//time_t  getNtpTime();           
//void    splitString(String, char, String*, uint8_t );
//void    writeSettings();
//===============END OF PROTOTYPES===========================================

//=======================================================================
// Display the human readable state of an A/C message if we can.
void dumpACInfo(decode_results *results) {
  String description = "";
  // If we got a human-readable description of the message, display it.
  if (description != "")  Serial.println("Mesg Desc.: " + description);
  
} // dumpACInfo()


//=======================================================================
bool startLearning() {
  waitForPulse = millis() + 30000;

  digitalWrite(GRN_LEDPIN, HIGH);
  _dThis = true;
  Debug("Start learning ..");
  
#if DECODE_HASH
  // Ignore messages with less than minimum on or off pulses.
  irrecv.setUnknownThreshold(kMinUnknownSize);
#endif  // DECODE_HASH
  irrecv.enableIRIn();  // Start the receiver


  while(millis() < waitForPulse) {
    yield();
    // Check if the IR code has been received.
    if (irrecv.decode(&results)) {
      // Display a crude timestamp.
      uint32_t now = millis();
      Debugf("\nTimestamp : %06u.%03u\n", now / 1000, now % 1000);
      if (results.overflow)
        Debugf("WARNING: IR code is too big for buffer (>= %d). "
               "This result shouldn't be trusted until this is resolved. "
               "Edit & increase kCaptureBufferSize.\n",
                                              kCaptureBufferSize);
      // Display the basic output of what we found.
      Debug(resultToHumanReadableBasic(&results));
      dumpACInfo(&results);  // Display any extra A/C info if we have it.
      yield();  // Feed the WDT as the text output can take a while to print.

      // Display the library version the message was captured with.
      Debug("Library   : v");
      Debugln(_IRREMOTEESP8266_VERSION_);
      Debugln();

      // Output RAW timing info of the result.
      Debugln(resultToTimingInfo(&results));
      yield();  // Feed the WDT (again)

      // Output the results as source code
      Debugln(resultToSourceCode(&results));
      Debugln("");  // Blank line between entries
      yield();  // Feed the WDT (again)

      irrecv.disableIRIn();  // End the receiver
      digitalWrite(GRN_LEDPIN, LOW);

      return true;
    }

    HttpServer.handleClient();
    webSocket.loop();

  } // while waitForPulse

  irrecv.disableIRIn();  // End the receiver
  digitalWrite(GRN_LEDPIN, LOW);
  Debugln(" .. time out!");
  return false;
  
} // startLearning()

//=======================================================================
void setup() {
  
  Serial.begin ( 115200 );
  lastResetReason = ESP.getResetReason();

  pinMode(IR_LEDPIN,  OUTPUT);
  pinMode(RED_LEDPIN, OUTPUT);
  pinMode(GRN_LEDPIN, OUTPUT);

  setTime(dateTime2Epoch(__DATE__, __TIME__));
  startWiFi();
  
  _dThis = true;
  Debugf("last Reset Reason [%s]\n", lastResetReason.c_str());
  _dThis = true;
  Debugf("Compile time [%04d-%02d-%02d @ %02d:%02d:%02d]\n", year(), month(), day(), hour(), minute(), second());
  startTelnet();
 
  for (int f=0; f<10; f++) {
    delay(200);
    digitalWrite(GRN_LEDPIN,  digitalRead(RED_LEDPIN));
    digitalWrite(RED_LEDPIN, !digitalRead(RED_LEDPIN));
  }
  digitalWrite(RED_LEDPIN, LOW);
  digitalWrite(GRN_LEDPIN, LOW);

/*  
 *   list all services with the cammand:
 *   dns-sd -B _arduino .
 *   dns-sd -B _http .
*/
  startMDNS(_HOSTNAME);
  _dThis = true;
  Debugln("addService('http', 'tcp' 80)");
  MDNS.addService("http", "tcp", 80);
  MDNS.port(80);  // webserver
  Debugln("addService('arduino', 'tcp' 81)");
  MDNS.addService("arduino", "tcp", 81);
  MDNS.port(81);  // webSockets

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  _dThis = true;
  if (!startNTP()) {
    _dThis = true;
    Debugln("ERROR!!! No NTP server reached!\n");
  }

  _dThis = true;
  if (!SPIFFS.begin()) {
    Debugln("SPIFFS Mount failed\n");           // Serious problem with SPIFFS 
  } else { 
    Debugln("SPIFFS Mount succesfull");
  }
  
  HttpServer.on("/ReBoot",  HTTP_POST, handleReBoot);
  HttpServer.on("/restAPI", HTTP_GET, handleRestAPI);
  HttpServer.on("/restapi", HTTP_GET, handleRestAPI);

  HttpServer.serveStatic("/",                   SPIFFS, "/IR_IoT_Remote.html");
  HttpServer.serveStatic("/IR_IoT_Remote.js",   SPIFFS, "/IR_IoT_Remote.js");
  HttpServer.serveStatic("/IR_IoT_Remote.css",  SPIFFS, "/IR_IoT_Remote.css");
  HttpServer.serveStatic("/FSexplorer.png",     SPIFFS, "/FSexplorer.png");

  HttpServer.on("/FSexplorer", HTTP_POST, handleFileDelete);
  HttpServer.on("/FSexplorer", handleRoot);
  HttpServer.on("/FSexplorer/upload", HTTP_POST, []() {
    HttpServer.send(200, "text/plain", "");
  }, handleFileUpload);

  //HttpServer.onNotFound([]() {
  //  if (!handleFileRead(HttpServer.uri()))
  //    HttpServer.send(404, "text/plain", "FileNotFound");
  //});
  HttpServer.onNotFound([]() {
    if (HttpServer.uri() == "/update") {
      HttpServer.send(200, "text/html", "/update" );
    } else {
      _dThis = true;
      Debugf("onNotFound(%s)\n", HttpServer.uri().c_str());
      if (HttpServer.uri() == "/") {
        reloadPage("/");
      }
    }
    if (!handleFileRead(HttpServer.uri())) {
      HttpServer.send(404, "text/plain", "FileNotFound");
    }
  });

  HttpUpdater.setup(&HttpServer); 
  HttpServer.begin();
  _dThis = true;
  Debugln( "HTTP server started\n" );

  _dThis = true;
  Debug("setup(): Start Time: ");
  Debugf("%02d:%02d:%02d\n", hour(), minute(), second());

  DS18B20.begin();
  
  // locate devices on the bus
  _dThis = true;
  Debugf("Locating DS18B20 sensors... Found [%d] devices\n", DS18B20.getDeviceCount());

  // search() looks for the next device. Returns 1 if a new address has been
  // returned. A zero might mean that the bus is shorted, there are no devices,
  // or you have already retrieved all of them. It might be a good idea to
  // check the CRC to make sure you didn't get garbage. The order is
  // deterministic. You will always get the same devices in the same order
  //
  // Must be called before search()
  //DS18B20_0 = null;
  oneWire.reset_search();
  _dThis = true;
  if (DS18B20.getDeviceCount() == 0) Debugln("Unable to find address for DS18B20's");

  inTemp0 = getInsideTemp(0);
  
  _dThis = true;
  if (!hasDS18B20sensor) {
    inTemp0  = 0;
    tn0      = 0;
    Debugln("setup(): no inside temperature sensors found!");
  } else {
    Debugln("setup(): has inside temperature sensor(s)");
    //inTemp0  = getInsideTemp(0);
    tn0     = inTemp0;
  }

  for (int f=0; f<6; f++) {
    delay(500);
    digitalWrite(RED_LEDPIN, !digitalRead(RED_LEDPIN));
  }
  digitalWrite(RED_LEDPIN, LOW);

  _dThis = true;
  Debugf("last Reset Reason [%s]\n", lastResetReason.c_str());

  // --- initialize -----
  readSettings();
  
  clientActiveTab       = TAB_UNKNOWN;
  upTimeSeconds         = millis() / 1000;
  nextSecond            = millis() + 1000;
  nextStateSend         = millis() + 2000;
  nextDS18B20PollTime   = millis() + 10000;
  
  _dThis = true;
  Debugln("================ done with setup() =====================\n");
  
} // setup()


//=======================================================================
void loop() {
  HttpServer.handleClient();
  MDNS.update();
  webSocket.loop();
  if (millis() > nextDS18B20PollTime) { 
    nextDS18B20PollTime = millis() + 30000;
    handleSensor();
    digitalWrite(GRN_LEDPIN, HIGH);
    delay(100);
    digitalWrite(GRN_LEDPIN, LOW);
  }

  handleKeyInput(); // menu

  if (millis() > nextSecond) {
    nextSecond += 1000;
    upTimeSeconds++;
  }
  if (millis() > nextStateSend) {
    nextStateSend += 2000;
  }

} // loop()


/***************************************************************************
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to permit
* persons to whom the Software is furnished to do so, subject to the
* following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
* OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
* THE USE OR OTHER DEALINGS IN THE SOFTWARE.
* 
***************************************************************************/
