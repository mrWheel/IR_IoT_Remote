/*
***************************************************************************  
**  Program  : WebSocketStuff, part of IR_IoT_Remote
**  Version  : v0.2.2
**
**  Copyright (c) 2019 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/

//===========================================================================================
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
//===========================================================================================
    String  text = String((char *) &payload[0]);
    char *  textC = (char *) &payload[0];
    String  pOut[5], pDev[5], pVal[5], words[10], jsonString;
    int8_t  deviceNr;

    switch(type) {
        case WStype_DISCONNECTED:
            _dThis = true;
            Debugf("[%u] Disconnected!\n", num);
            isConnected = false;
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                if (!isConnected) {
                  _dThis = true;
                  Debugf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
                  isConnected = true;
                  webSocket.sendTXT(num, "{\"msgType\":\"ConnectState\",\"Value\":\"Connected\"}");
                  //sendDevInfo();
                }
        
            }
            break;
        case WStype_TEXT:

            _dThis = true;
            Debugf("[%u] Got message: [%s]\n", num, payload);
            splitString(text, ':', words, 10);
            sprintf(cMsg, "{\"msgType\":\"Temp\",\"Value\":\"%3.1f&deg;C\"}", DS18B20Temp);
            webSocket.sendTXT(num, cMsg);

            // send data to all connected clients
            // webSocket.sendTXT(num, "message here");
            if (text.indexOf("tabControl") > -1) {  // Control tab is clicked
              if (clientActiveTab == TAB_EDIT) {
                deleteButtons();
              }
              clientActiveTab   = TAB_CONTROL;
              newTab            = true;
              waitForPulse      = 0;
              Debugf("websocket.sendTXT(%d, sendAvailableButtons(BUTTONS))\n", num);
              webSocket.sendTXT(num, sendAvailableButtons("BUTTONS").c_str());

            } else if (text.indexOf("tabLearn") > -1) {
              clientActiveTab   = TAB_LEARN;
              newTab            = true;

            } else if (text.indexOf("startLearning") > -1) {
              if (clientActiveTab  == TAB_LEARN) {
                if (startLearning()) {
                  _dThis = true;
                  Debugf("websocket.sendTXT(%d, receivedPulse)\n", num);
                  webSocket.sendTXT(num, "receivedPulse");
                } else {
                  webSocket.sendTXT(num, "tabControl");
                }
              }
            
            } else if (text.indexOf("tabEdit") > -1) {
              clientActiveTab   = TAB_EDIT;
              newTab            = true;
              waitForPulse      = 0;
              Debugf("websocket.sendTXT(%d, sendAvailableButtons(EDITS))\n", num);
              webSocket.sendTXT(num, sendAvailableButtons("EDITS").c_str());
              
            } else if (text.indexOf("editField") > -1) {
              editField(words[1], words[2], words[3]);
              webSocket.sendTXT(num, "reloadEdit");
              
            } else if (text.indexOf("saveSettings") > -1) {
              writeSettings();
            
            } else if (text.indexOf("saveNewPulse") > -1) {
                _dThis = true;
                Debugf("[%s] => seq[%s], Label[%s] with Filename[%s]\n", words[0].c_str(), words[1].c_str(), words[2].c_str(), words[3].c_str());
                words[3] = normalizePlsName(words[3]);
                writeRawData(words[3],  &results);
                int16_t bPos = findPls(words[3].c_str());
                if (bPos < 0) { // not found .. new pulse
                  if (words[1].toInt() < 1 || words[1].toInt() > 99)
                        Buttons[maxButtons].Sequence = 1;
                  else  Buttons[maxButtons].Sequence = words[1].toInt();
                  strcpy(Buttons[maxButtons].buttonLabel, words[2].c_str());
                  strcpy(Buttons[maxButtons].pulseFile, words[3].c_str());
                  maxButtons++;
                  writeSettings();
                  
                } else {
                  //Buttons[bPos].Sequence     = bPos;
                  strcpy(Buttons[bPos].buttonLabel, words[2].c_str());
                  strcpy(Buttons[bPos].pulseFile, words[3].c_str());
                  writeSettings();
                  
                }
                Debugf("websocket.sendTXT(%d, saveDone)\n", num);
                webSocket.sendTXT(num, "saveDone");

            } else if (text.indexOf("IRSEND") > -1) {
              _dThis = true;
              Debugf("Send IR from file [%s]\n", words[1].c_str());
              noPulses = readRawData(words[1], rawData);
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
                noPulses = -1;
                nextStateSend = millis() + 10000;
              }
            }
                        
            if (text.indexOf("getDevInfo") > -1) {
              jsonString  = "{";
              jsonString += "\"msgType\": \"devInfo\"";
              jsonString += ", \"devName\": \"" + String(_HOSTNAME) + " \"";
              jsonString += ", \"devIPaddress\": \"" + WiFi.localIP().toString() + " \"";
              jsonString += ", \"devVersion\": \" [" + String(_FW_VERSION) + "]\"";
              jsonString += "}";
              _dThis = true;
              Debugf("websocket.sendTXT(%d, %s)\n", num, jsonString.c_str());
              webSocket.sendTXT(num, jsonString);
            } else if (text.startsWith("Button")) {
              //splitString(text, ':', words, 10);
            }
            break;
                        
    } // switch(type)
    
} // webSocketEvent()

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
