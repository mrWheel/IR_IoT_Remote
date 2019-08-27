/*
***************************************************************************  
**  Program  : restAPI, part of OR_IoT_Remote
**  Version  : v0.2.2
**
**  Copyright (c) 2019 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/

//=======================================================================
void handleRestAPI() {
//=======================================================================
  String pName;
  String pVal;

  for (uint8_t i = 0; i < HttpServer.args(); i++) {
    pName  = HttpServer.argName(i);
    pName.toUpperCase();
    pVal   = HttpServer.arg(i);
    pVal.toUpperCase();
    _dThis = true;
    Debugf("[%d] [%s] => [%s]\r\n", i, pName.c_str(), pVal.c_str());
    if (pName == "GET") i = 99; 
  }

  if (pName == "GET") {
    if (pVal.indexOf("INFO") > -1) {
      sendDeviceInfo();
    } else if (pVal.indexOf("TEMP") > -1) {
      sendTemperature();      
    } else if (pVal.indexOf("LIST") > -1) {
      sendButtons();      
    }
  } else if (pName.indexOf("PUSH") > -1) {
    pushButton(pVal);

  } else {
    listAPI();
  }

} // restAPI()


//=======================================================================
void listAPI() {
//=======================================================================
String wsString;
  
//-IR_IoT_Remote API----------------------------------------------------------
  wsString  = "{";
//-Device Temperature---------------------------------------------------------
  wsString += "\r\n  \"GET\":\"DevInfo | Temp | List\"";
  wsString += ",\r\n  \"PUSH\":\"Button Name\"";

  wsString += "\r\n}\r\n";
  
  HttpServer.send(200, "application/json", wsString);
  _dThis = true;
  Debugln("listAPI(): send JSON string\r\n");

} // listAPI()


//=======================================================================
void pushButton(String buttonId) {
//=======================================================================
  uint8_t pulseNr;
  
  _dThis = true;
  if (buttonId.length() > 2) {
    Debugf("Input Name [%s]\n", String(buttonId).c_str());   
    pulseNr = findLabel(buttonId) ;
  } else if (buttonId.toInt() >= 0) {
    pulseNr = buttonId.toInt();
    Debugf("Input button #[%d]\n", pulseNr);

  } else if (buttonId.toInt() < 0) {
    noPulses = -1;
    sprintf(cMsg, "Input button #[%d]\r\nERROR\r\n", buttonId.toInt());
    _dThis = true;
    Debugln(cMsg);
    HttpServer.send(200, "text/plain", cMsg);
    return;
  }

  if (pulseNr >= maxButtons || pulseNr < 0) {
    noPulses = -1;
    HttpServer.send(200, "text/plain", "pulseFile not found!\r\nERROR\r\n");
    return;
  }

  noPulses = readRawData(Buttons[pulseNr].pulseFile, rawData);

  if (noPulses < 1) {
    noPulses = -1;
    HttpServer.send(200, "text/plain", "pulseFile has zero pulses!\r\nERROR\r\n");
    return;
  }
  
  sprintf(cMsg, "[%s] -> pulseNr[%d] => pulseFile[%s] (%d pulses)\r\nOK\r\n", String(buttonId).c_str()
                                                        , pulseNr
                                                        , Buttons[pulseNr].pulseFile
                                                        , noPulses);
  _dThis = true;
  Debugln(cMsg);
  HttpServer.send(200, "text/plain", cMsg);

  _dThis = true;
  Debugf("Send IR from file [%s]\n", Buttons[pulseNr].pulseFile);
  if (noPulses > 0) {
    digitalWrite(RED_LEDPIN, HIGH);
    Debugf("Sending [%d] pulses .. ", noPulses);
    for (int s=1; s < 6; s++) {
      Debugf("%d ", s);
      irsend.sendRaw(rawData, noPulses, 38);  // Send a raw data capture at 38kHz.
      delay(100);
    }
    digitalWrite(RED_LEDPIN, LOW);
    Debugln();
  }
  noPulses = -1;
  nextStateSend = millis() + 10000;

} // pushButton()


//=======================================================================
void sendDeviceInfo() {
//=======================================================================
String wsString;
  
//-Slimme Meter Info----------------------------------------------------------
  wsString  = "{";
//-Device Info-----------------------------------------------------------------
  wsString += "\r\n \"Author\":\"Willem Aandewiel (www.aandewiel.nl)\"";
  wsString += "\r\n,\"FwVersion\":\""         + String( _FW_VERSION ) + "\"";
  wsString += "\r\n,\"Compiled\":\""          + String( __DATE__ ) 
                                            + String( "  " )
                                            + String( __TIME__ ) + "\"";
  wsString += "\r\n,\"FreeHeap\":\""          + String( ESP.getFreeHeap() ) + "\"";
  wsString += "\r\n,\"ChipID\":\""            + String( ESP.getChipId(), HEX ) + "\"";
  wsString += "\r\n,\"CoreVersion\":\""       + String( ESP.getCoreVersion() ) + "\"";
  wsString += "\r\n,\"SdkVersion\":\""        + String( ESP.getSdkVersion() ) + "\"";
  wsString += "\r\n,\"CpuFreqMHz\":\""        + String( ESP.getCpuFreqMHz() ) + "\"";
  wsString += "\r\n,\"SketchSize\":\""        + String( (ESP.getSketchSize() / 1024.0), 3) + "kB\"";
  wsString += "\r\n,\"FreeSketchSpace\":\""   + String( (ESP.getFreeSketchSpace() / 1024.0), 3 ) + "kB\"";

  if ((ESP.getFlashChipId() & 0x000000ff) == 0x85) 
        sprintf(cMsg, "%08X (PUYA)", ESP.getFlashChipId());
  else  sprintf(cMsg, "%08X", ESP.getFlashChipId());
  wsString += "\r\n,\"FlashChipID\":\""       + String(cMsg) + "\"";  // flashChipId
  wsString += "\r\n,\"FlashChipSize\":\""     + String( (float)(ESP.getFlashChipSize() / 1024.0 / 1024.0), 3 ) + "MB\"";
  wsString += "\r\n,\"FlashChipRealSize\":\"" + String( (float)(ESP.getFlashChipRealSize() / 1024.0 / 1024.0), 3 ) + "MB\"";
  wsString += "\r\n,\"FlashChipSpeed\":\""    + String( (float)(ESP.getFlashChipSpeed() / 1000.0 / 1000.0) ) + "MHz\"";

  FlashMode_t ideMode = ESP.getFlashChipMode();
  wsString += "\r\n,\"FlashChipMode\":\""    + String( flashMode[ideMode] ) + "\"";
  wsString += "\r\n,\"BoardType\":";
#ifdef ARDUINO_ESP8266_NODEMCU
    wsString += "\"ESP8266_NODEMCU\"";
#endif
#ifdef ARDUINO_ESP8266_WEMOS_D1R1
    wsString += "\"Wemos D1 R1\"";
#endif
#ifdef ARDUINO_ESP8266_GENERIC
    wsString += "\"ESP8266_GENERIC\"";
#endif
#ifdef ARDUINO_ESP8266_ESP12
    wsString += "\"ESP8266_ESP12\"";
#endif
#ifdef ESP8266_ESP12
    wsString += "\"ESP8266_ESP12(?)\"";
#endif
  wsString += "\r\n,\"SSID\":\""              + String( WiFi.SSID() ) + "\"";
//wsString += "\r\n,\"PskKey\":\""            + String( WiFi.psk() ) + "\"";    // uncomment if you want to see this
  wsString += "\r\n,\"IpAddress\":\""         + WiFi.localIP().toString()  + "\"";
  wsString += "\r\n,\"WiFiRSSI\":\""          + String(WiFi.RSSI())  + "\"";
  wsString += "\r\n,\"Hostname\":\""          + String( _HOSTNAME ) + "\"";
//wsString += "\r\n,\"upTime\":\""            + String( upTime() ) + "\"";
  wsString += "\r\n,\"lastReset\":\"" + ESP.getResetReason() + "\"";
  wsString += "\r\n}\r\n";
  
  HttpServer.send(200, "application/json", wsString);
  _dThis = true;
  Debugln("sendDataDeviceInfo(): send JSON string\r\n");

} // sendDeviceInfo()


//=======================================================================
void sendTemperature() {
//=======================================================================
String wsString;
  
//-IR_IoT_Remote--------------------------------------------------------------
  wsString  = "{";
//-Device Temperature---------------------------------------------------------
  wsString += "\r\n  \"Hostname\":\""          + String( _HOSTNAME ) + "\"";
  wsString += ",\"Temperature\":\""          + String( DS18B20Temp ) + "\"";

  wsString += "\r\n}\r\n";
  
  HttpServer.send(200, "application/json", wsString);
  _dThis = true;
  Debugln("sendDataTemperature(): send JSON string\r\n");

} // sendTemperature()


//=======================================================================
void sendButtons() {
//=======================================================================
String wsString;
char comma = ' ';
  
//-IR_IoT_Remote--------------------------------------------------------------
  wsString  = "{";
//-Device List of Buttons-----------------------------------------------------
  wsString += "\r\n  \"Hostname\":\""+ String( _HOSTNAME ) + "\",";
  wsString += "\r\n  \"Buttons\": [";
  for(int8_t ln=0; ln < maxButtons; ln++) {
    if (Buttons[ln].buttonLabel != "") {
      wsString += "\r\n     "+String(comma)+"{\"Name\":\""+String(Buttons[ln].buttonLabel) + "\"";
    //wsString += ",\"Num\":\""+String(Buttons[ln].Sequence) + "\"";
      wsString += ",\"File\":\""+String(Buttons[ln].pulseFile) + "\"}";
      comma = ',';
    }
  }
  wsString += "\r\n  ]\r\n}\r\n";
  
  HttpServer.send(200, "application/json", wsString);
  _dThis = true;
  Debugln("sendDataTemperature(): send JSON string\r\n");

} // sendButtons()



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
