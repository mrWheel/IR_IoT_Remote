/*
***************************************************************************  
**  Program  : insideTemp, part of IR_IoT_Remote
**  Version  : v0.2.1
**
**  Copyright (c) 2019 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
 *
 * deze functie leest de temperatuur uit de DS18B20 sensor
 * needs:
 * #include <OneWire.h>
 * #include <DallasTemperature.h>
 * and:
 * // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
 *    OneWire oneWire(ONE_WIRE_PIN);
 * // Pass our oneWire reference to Dallas Temperature. 
 *    DallasTemperature DS18B20(&oneWire);
 *
*/
#define TEMPERATURE_PRECISION 12    // 9, 10, 11 or 12 bits
#define TEMPERATURE_ERROR     1.00  // measurements are to high..

float getInsideTemp(uint8_t DS18B20DevNr) {
    
  char  fTmp[10], cMsg[20];
  
  if (DS18B20.getDeviceCount() > 0) {
  //for (int D=0; D < DS18B20.getDeviceCount(); D++) {
      sprintf(cMsg, "%X", DS18B20addr[DS18B20DevNr]);
      DS18B20.setResolution(DS18B20addr[DS18B20DevNr], TEMPERATURE_PRECISION);
      delay(50);
      DS18B20.requestTemperatures();
      delay(100);
      DS18B20Temp = DS18B20.getTempCByIndex(DS18B20DevNr);
      //dtostrf(DS18B20Temp, 6, 2, fTmp);  
      _dThis = true;
      Debugf("getInsideTemp(%d): DS18B20 (%s) \t[%s]\n", DS18B20DevNr, cMsg, String(DS18B20Temp, 1).c_str() );
      hasDS18B20sensor = true;    
  //}  
  } else {
    hasDS18B20sensor = false;      
  }
  if (DS18B20Temp <= -127 || DS18B20Temp >= 126) {
    DS18B20Temp = lastTemp;  // fallback to last known inTemp0
    _dThis = true;
    Debugln("getInsideTemp(): Error reading inside Temperature!");
  } else {
    lastTemp = DS18B20Temp;
    Debugf("getInsideTemp(%d): insideTemperature \t[%s]\n", DS18B20DevNr, String(lastTemp, 2).c_str());
  }
  
  return DS18B20Temp;
    
}   // getInsideTemp()


// function to print a device address
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    // zero pad the address if necessary
    //if (deviceAddress[i] < 10) Debug("0");
    _dThis = true;
    Debug(deviceAddress[i]);
    Debug(":");
  }
}


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
