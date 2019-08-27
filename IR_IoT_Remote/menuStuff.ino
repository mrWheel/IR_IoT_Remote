/*
***************************************************************************  
**  Program  : menuStuff, part of IR_IoT_Remote
**  Version  : v0.2.1
**
**  Copyright (c) 2019 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/

//=======================================================================
void showStatus() {
  char cMsg[150];

  SPIFFS.info(SPIFFSinfo);

  _dThis = false;  
  if (DS18B20.getDeviceCount() > 0) {
    Debugln("\n----------------------------------------------------------------");
    Debugf("#> inside Temperature  [%s]*C\n", String(DS18B20Temp, 2).c_str());
    Debugln("----------------------------------------------------------------");
  }
  Debug("\n==================================================================");
  Debug(" \n               (c)2019 by [Willem Aandewiel");
  Debug("]\n         Firmware Version [");  Debug( _FW_VERSION );
  Debug("]\n                 Compiled [");  Debug( __DATE__ ); 
                                              Debug( "  " );
                                              Debug( __TIME__ );
  Debug("]\n               Flash Size [*4M (2M SPIFFS)*");  
  Debug("]\n        last Reset Reason [");  Debug( lastResetReason );
  Debug("]\n                 FreeHeap [");  Debug( String(ESP.getFreeHeap()).c_str() );

  sprintf(cMsg, "%08X", String( ESP.getChipId(), HEX ).c_str() );

  Debug("]\n                  Chip ID [");  Debug( cMsg );
  Debug("]\n             Core Version [");  Debug( String( ESP.getCoreVersion() ).c_str() );
  Debug("]\n              SDK Version [");  Debug( String( ESP.getSdkVersion() ).c_str() );
  Debug("]\n           CPU Freq (MHz) [");  Debug( String( ESP.getCpuFreqMHz() ).c_str() );
  FlashMode_t ideMode = ESP.getFlashChipMode();
  Debug("]\n          Flash Chip Mode [");  Debug( flashMode[ideMode] );
  Debug("]\n               Board type [");
#ifdef ARDUINO_ESP8266_NODEMCU
    Debug("ESP8266_NODEMCU");
#endif
#ifdef ARDUINO_ESP8266_WEMOS_D1R1
    Debug("Wemos D1 R1");
#endif
#ifdef ARDUINO_ESP8266_GENERIC
    Debug("ESP8266_GENERIC");
#endif
#ifdef ARDUINO_ESP8266_ESP12
    Debug("ESP8266_ESP12");
#endif
#ifdef ESP8266_ESP12
    Debug("ESP8266_ESP12(?)");
#endif
  Debug("]\n==================================================================");

  if ((ESP.getFlashChipId() & 0x000000ff) == 0x85) 
        sprintf(cMsg, "%08X (PUYA)", ESP.getFlashChipId());
  else  sprintf(cMsg, "%08X", ESP.getFlashChipId());
  Debug(" \n            Flash Chip ID [");  Debug( cMsg );
  Debug("]\n     Flash Chip Size (kB) [");  Debug( String( ESP.getFlashChipSize() / 1024 ).c_str() );
  Debug("]\nFlash Chip Real Size (kB) [");  Debug( String( ESP.getFlashChipRealSize() / 1024 ).c_str() );
  Debug("]\n         Sketch Size (kB) [");  Debug( String( ESP.getSketchSize() / 1024.0 ).c_str() );
  Debug("]\n   Free Sketch Space (kB) [");  Debug( String( ESP.getFreeSketchSpace() / 1024.0 ).c_str() );
  Debug("]\n         SPIFFS Size (kB) [");  Debug( SPIFFSinfo.totalBytes / 1024 );
  Debug("]\n    Flash Chip Speed (MHz)[");  Debug( String( ESP.getFlashChipSpeed() / 1000 / 1000 ).c_str() );

  Debug("]\n");

  Debug("==================================================================");
  Debug(" \n                     SSID [");  Debug( WiFi.SSID() );
  Debug("]\n                  PSK key [");  Debug( WiFi.psk() );
  Debug("]\n               IP Address [");  Debug( WiFi.localIP().toString() );
  Debug("]\n                 Hostname [");  Debug( _HOSTNAME );
//Debug("]\n   Firmware Update server [");  Debug( settingUpdateURL );
//Debug("]\n                     See: [https://www.grc.com/fingerprints.htm");
//Debug("]\n Update Server Fingeprint [");  Debug( settingFingerPrint ); 
  
  Debug("]\n                   upTime [");  Debug( uptime() );
  Debug("]\n");
  Debug("==================================================================\n");
  Debug("number of DS18B20 sensors [");  Debug( String(DS18B20.getDeviceCount()) );
  Debug("]\n");
  for (int D=0; D < DS18B20.getDeviceCount(); D++) {
    //DS18B20.setResolution(DS18B20addr[D], TEMPERATURE_PRECISION);
    //delay(100);
      DS18B20.requestTemperatures();
      delay(100);
      float DS18B20Temp = DS18B20.getTempCByIndex(D);
      sprintf(cMsg, "%X", DS18B20addr[D]);
      Debugf("       Address Device (%d) [", D);  Debug( cMsg );
      Debug("]\n");
      Debugf("   Temperature Device (%d) [", D);  Debug( String(DS18B20Temp, 2) );
      Debug("]\n");
  }
  //Debugln("]");
  Debugln("==================================================================\n");
  TelnetStream.flush();
  
} // showStatus()


//=======================================================================
void updateFirmware() {

    _dThis = true;
    Debugln("Sorry, server-update crashes all the time ... Skipping!");
    /****
    //t_httpUpdate_return ret = ESPhttpUpdate.update(settingUpdateURL.c_str());
    t_httpUpdate_return  ret = ESPhttpUpdate.update(settingUpdateURL.c_str(), "", settingFingerPrint.c_str());

    _dThis = true;
    Debugln("Done Updating Firmware ..");

    _dThis = true;
    switch (ret) {
      case HTTP_UPDATE_FAILED:
        Debugf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        break;

      case HTTP_UPDATE_NO_UPDATES:
        Debugln("HTTP_UPDATE_NO_UPDATES");
        break;

      case HTTP_UPDATE_OK:
        Debugln("HTTP_UPDATE_OK");
        break;
    }
    ***/
    Debugln();

} // updateFirmware()


//=======================================================================
void listButtons() {
  _dThis = false;
  Debugln("----------------------------------------------------------------------------------------");
  for (int l=0; l < maxButtons; l++) {
    _dThis = false;
    Debugf("[%2d] => Seq[%2d], Label[%-20.20s], pulseFile[%-32.32s]\n" 
                      , l
                      , Buttons[l].Sequence
                      , Buttons[l].buttonLabel
                      , Buttons[l].pulseFile);
  }

  _dThis = false;
  Debugln("----------------------------------------------------------------------------------------");

} // listButtons()


//=======================================================================
void handleKeyInput() {
  
  while (TelnetStream.available() > 0 || Serial.available() > 0) {
    yield();
    if (TelnetStream.available() > 0) {
      inChar = (char)TelnetStream.read();
    } else if (Serial.available() > 0) {
      inChar = (char)Serial.read();
    }
    
    switch(inChar) {
      case 'b':
      case 'B':     showStatus();
                    break;
      case 'i':
      case 'I':     DS18B20Temp = getInsideTemp(0);
                    Debugf("Inside temperature is %s*C\n", String(DS18B20Temp, 1).c_str());
                    break;
      case 'l':
      case 'L':     listButtons();
                    break;
      case 'R':     ESP.reset();
                    break;
      case 's':
      case 'S':     listSPIFFS("/");
                    listSPIFFS("/pls/");
                    infoSPIFFS();
                    break;
      case 'U':     updateFirmware();
                    break;
      default:      _dThis = false;
                    Debugln("\nCommandos are:\n");
                    Debugln("   B - Board, Build info & System Status");
                    Debugln("   I - Inside Temperature");
                    Debugln("   L - list Buttons");
                    Debugln("  *R - Reboot");
                    Debugln("   S - list SPIFFS");
                    Debugln("  *U - update Firmware");
                    Debugln(" ");

    } // switch()
    while (TelnetStream.available() > 0) {
     yield();
     (char)TelnetStream.read();
    }
    while (Serial.available() > 0) {
     yield();
     (char)Serial.read();
    }
  }
  
} // handleKeyInput()


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
