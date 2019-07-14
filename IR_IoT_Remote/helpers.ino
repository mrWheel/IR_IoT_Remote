/*
***************************************************************************  
**  Program  : helpers, part of IR_IoT_Remote
**  Version  : v0.1.0
**
**  Copyright (c) 2019 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/


//=======================================================================
String dateToString(uint32_t dt) {
//=======================================================================
  char ds[30];

  sprintf(ds, "%04d-%02d-%02d %02d:%02d", year(dt), month(dt), day(dt)
                                        , hour(dt), minute(dt));

  return String(ds);                                      
  
} //  dateToString()


//=======================================================================
String uptime() {
//=======================================================================
  char calcUptime[20];

  sprintf(calcUptime, "%d(d) %02d:%02d", int((upTimeSeconds / (60 * 60 * 24)) % 365)
                                       , int((upTimeSeconds / (60 * 60)) % 24)
                                       , int((upTimeSeconds / (60)) % 60));

  return calcUptime;

} // uptime()


//=======================================================================
void splitString(String inStrng, char delimiter, String wOut[], uint8_t maxWords) {
//=======================================================================
  int8_t inxS = 0, inxE = 0, wordCount = 0;

    while(inxE < inStrng.length() && wordCount < maxWords) {
      inxE  = inStrng.indexOf(delimiter, inxS);             //finds location of first ,
      wOut[wordCount] = inStrng.substring(inxS, inxE);  //captures first data String
      wOut[wordCount].trim();
      inxS = inxE;
      inxS++;
      wordCount++;
    }
    if (inxS < inStrng.length()) {
      wOut[wordCount] = inStrng.substring(inxS, inStrng.length());  //captures first data String      
    }
    
} // splitString()


//=======================================================================
void handleSensor() {
//=======================================================================
    if (!hasDS18B20sensor) {
      // ===== verzin wat data =====
      tn0 = -1;
      tn1 = -1;
    } else {
      tn0 = getInsideTemp(0);   
      tn1 = getInsideTemp(1);   
      _dThis = true;
      Debugf("loop(): Sensor: T=[%s/%s], newT=[%s/%s] \n", String(inTemp0).c_str(), String(inTemp1).c_str()
                                                        , String(tn0).c_str(), String(tn1).c_str());
    }
    
} // handleSensor()



//=======================================================================
String normalizePlsName(String plsFileName) {
//=======================================================================
  char    normName[32];
  int8_t  normChars = 0;
  normName[normChars] = '\0';

  plsFileName.replace("\n", "");
  plsFileName.replace("\r", "");
  plsFileName.replace("/pls/", "");
  plsFileName.replace(".pls", "");
  plsFileName.replace(" ", "");
  plsFileName.replace("/", "");
  plsFileName.toLowerCase();
  for (int16_t c = 0; c < plsFileName.length(); c++) {
    if (   (plsFileName[c] >= '0' && plsFileName[c] <= '9') 
        || (plsFileName[c] >= 'a' && plsFileName[c] <= 'z')
          ) { 
        normName[normChars++] = plsFileName[c];
        normName[normChars] = '\0'; 
    }
  }
  plsFileName = "/pls/" + String(normName) + ".pls";
  
  return plsFileName;

} // normalizePlsName()


//=======================================================================
int8_t findPls(String plsFileName) {
//=======================================================================
  for(int s = 0; s < maxButtons; s++) {
    _dThis = true;
    Debugf("Test Button[%d] ..", s);
    if (Buttons[s].pulseFile == plsFileName.c_str()) {
      Debugf(" .. found[%s] at position [%d]\n", plsFileName.c_str(), s);
      return s;
    }
    Debugln(".");
  }
  return -1;

} // findPls()


//=======================================================================
String sendAvailableButtons(char *FUNCT) {
//=======================================================================
  String  availableButtons = "";

  availableButtons = String(FUNCT);
  for(int8_t ln=0; ln < maxButtons; ln++) {
    _dThis = true;
    Debugf("[%2d] => [%2d]->[%s]/[%s]\n", ln, Buttons[ln].Sequence, Buttons[ln].buttonLabel, Buttons[ln].pulseFile);
    if (Buttons[ln].buttonLabel != "") {
      availableButtons += ":"+String(Buttons[ln].Sequence);
      availableButtons += ","+String(Buttons[ln].buttonLabel);
      availableButtons += ","+String(Buttons[ln].pulseFile);
    }
  } // for ln ..

  return availableButtons;
  
} // sendAvailableButtons()


//=======================================================================
int8_t findLowestSeq() {
//=======================================================================
int8_t  lowSeq = 99;

  // find lowest seq.
  for(int8_t ln=0; ln < maxButtons; ln++) {
    if (Buttons[ln].Sequence < lowSeq) {
      lowSeq = Buttons[ln].Sequence;
    }
  }

  return lowSeq;

} // findLowestSeq()


//=======================================================================
int8_t findHighestSeq() {
//=======================================================================
int8_t  highSeq = -1;

  // find highest seq.
  for(int8_t ln=0; ln < maxButtons; ln++) {
    if (Buttons[ln].Sequence > highSeq) {
      highSeq = Buttons[ln].Sequence;
    }
  }

  return highSeq;

} // findHighestSeq()


//=======================================================================
void editField(String fldNum, String oldVal, String newVal) {
//=======================================================================
  int seq;
  
  _dThis = true;
  Debugf("index[%s], oldVal[%s], newVal[%s]\n", fldNum.c_str(), oldVal.c_str(), newVal.c_str());
  
  if (fldNum.indexOf("index") > -1) { // index of button
    seq = fldNum.substring(5).toInt();
    Buttons[seq].Sequence = newVal.toInt();
    _dThis = true;
    Debugf("index[%d] => newSeq[%d]\n", seq, Buttons[seq].Sequence);
  }
  if (fldNum.indexOf("label") > -1) { // label of button
    seq = fldNum.substring(5).toInt();
    strcpy(Buttons[seq].buttonLabel, newVal.c_str());
    _dThis = true;
    Debugf("index[%d] => newLabel[%s]\n", seq, Buttons[seq].buttonLabel);
  }
  _dThis = true;
  Debugf("index[%d] => Seq[%d], Label[%s], file[%s]\n", seq, Buttons[seq].Sequence, Buttons[seq].buttonLabel, Buttons[seq].pulseFile);
  writeSettings();

} // editField()


//=======================================================================
void sortButtons() {
//=======================================================================
  int x, y;

    for (int8_t y = 0; y < maxButtons - 1; y++) {
        yield();
        for (int8_t x = y + 1; x < maxButtons; x++)  {
            _dThis = true;
            //Debugf("y[%d], x[%d] => seq[x][%d] ", y, x, Buttons[x].Sequence);
            if (Buttons[x].Sequence < Buttons[y].Sequence)  {
                //Debugf("< seq[y][%d]", Buttons[y].Sequence);
                dataStruct temp = Buttons[y];
                Buttons[y] = Buttons[x];
                Buttons[x] = temp;
            } /* end if */
            //Debugln();
        } /* end for */
    } /* end for */
 
} // sortButtons()


//=======================================================================
void writeSettings() {
//=======================================================================
  uint32_t  maxTime = millis() + 10000;
  bool      bDone = false;

  _dThis = true;
  Debugf(" %s .. \n", String(_SETTINGS_FILE).c_str());


  sortButtons();
  listButtons();

  File file = SPIFFS.open(_SETTINGS_FILE, "w");
  
    for(int8_t r=0; r < maxButtons; r++) {
        _dThis = true;
        Debugf("[%2d] => [%2d][%s]/[%s]\n", r, Buttons[r].Sequence, Buttons[r].buttonLabel, Buttons[r].pulseFile);
        if (Buttons[r].buttonLabel != "") {
          file.print(Buttons[r].Sequence);
          file.print(":");
          file.print(Buttons[r].buttonLabel);
          file.print(":");
          file.print(Buttons[r].pulseFile);
          file.println();
        } // empty label

    } // for ln ..

  file.close();  

  readSettings();
  
  _dThis = true;
  Debugln(" .. done");

} // writeSettings()


//=======================================================================
void readSettings() {
//=======================================================================
  String sTmp;
  String words[10];
  int8_t r = 0;
  bool   fileError = false;

  maxButtons = 0;
  _dThis = true;
  Debugf(" %s ..", String(_SETTINGS_FILE).c_str());

  if (!SPIFFS.exists(_SETTINGS_FILE)) {
    _dThis = false;
    Debugln(" .. done (file not found!)");
    return;
  }

  File file = SPIFFS.open(_SETTINGS_FILE, "r");

  _dThis = false;
  Debugln();
  while(file.available()) {
    yield();
    r++;
    sTmp                = file.readStringUntil('\n');
    sTmp.replace("\r", "");
    _dThis = true;
    Debugf("[%s] (%d)\n", sTmp.c_str(), sTmp.length());
    splitString(sTmp, ':', words, 10);
    if (SPIFFS.exists(words[2])) {
      Buttons[maxButtons].Sequence    = words[0].toInt();
      strcpy(Buttons[maxButtons].buttonLabel, words[1].c_str());
      strcpy(Buttons[maxButtons].pulseFile, words[2].c_str());
      maxButtons++;
    } else {
      _dThis = true;
      Debugf("[%s] has [%s] as entry [%d] but file does not exist!\n", _SETTINGS_FILE, words[2].c_str(), r); 
      fileError = true;
    }
  } // while available()

  Debugln();
  
  file.close();  
  _dThis = true;
  Debugln(" .. done");
  DebugFlush();

  if (fileError) {
    writeSettings();
  }

} // readSettings()


//=======================================================================
void writeRawData(String fName, const decode_results *results) {
//=======================================================================

  _dThis = true;
  Debugf("writeRawData(%s): number of pulses [%ld]\n", fName.c_str(), (results->rawlen - 1));
  if (results->rawlen <= 1) {
     _dThis = true;
     Debugln("Nothing to write ..");
     return;
  }
  fName = normalizePlsName(fName);
  /**
  fName.replace("\n", "");
  fName.replace("\r", "");
  fName.replace("/pls/", "");
  fName.replace(".pls", "");
  fName.replace(" ", "");
  fName.replace("/", "");
  for (int16_t c = 0; c < fName.length(); c++) {
    if (   (fName[c] >= '0' && fName[c] <= '9') 
          || (fName[c] >= 'A' && fName[c] <= 'Z')
          || (fName[c] >= 'a' && fName[c] <= 'z')
          ) { 
        //Serial.print((char)fileName[c]); 
    } else {
        fName[c] = '*';
        //Serial.print((char)fileName[c]); 
    }
  }
  fName = "/pls/" + fName + ".pls";
  **/
  
  File file = SPIFFS.open(fName, "w");
  uint16_t  pulsNr = 0;

  _dThis = false;
  for (uint16_t i = 1; i < results->rawlen; i++) {
    uint32_t usecs;
    yield();
    usecs = results->rawbuf[i] * RAWTICK;
    file.print(usecs);
    file.print(";");
    if (pulsNr >= (results->rawlen - 1))
         Debugf("%ld ", usecs);
    else Debugf("%ld, ", usecs);
    if (i%20 == 0) Debugln(" ");
    pulsNr++;
  }

  file.close();  

  Debugln();
  _dThis = true;
  Debugf("writeRawData(): saved [%s] => [%ld] pulses\n", fName.c_str(), pulsNr);

} // writeRawData()


//=======================================================================
uint16_t readRawData(String fName, uint16_t inData[]) {
//=======================================================================

  uint16_t  pulsNr = 0;

  _dThis = true;
  Debugf("readRawData(%s): \n", fName.c_str());
  
  memset(inData, 0, sizeof(inData));
  File file = SPIFFS.open(fName, "r");
    
  while(file.available()) {
    yield();
    inData[pulsNr] = (uint16_t)file.readStringUntil(';').toInt();
    Debugf("[%-3d:%5d], ", pulsNr, inData[pulsNr]);
    if (pulsNr%10 == 0) Debugln(" ");
    pulsNr++;
  }
  Debugln("");
  
  file.close();  
  _dThis = true;
  Debugf("readRawData(): read [%ld] pulses\n", pulsNr);
  
  return pulsNr;

} // readRawData()

//===========================================================================================
int32_t freeSpace() {
//===========================================================================================
  int32_t space;
  
  SPIFFS.info(SPIFFSinfo);

  space = (int32_t)(SPIFFSinfo.totalBytes - SPIFFSinfo.usedBytes);

  return space;
  
} // freeSpace()

//===========================================================================================
void listSPIFFS(char * lDir) {
//===========================================================================================
  bool onlyRoot = false;
  
  if (lDir[0] == '\0') { lDir[0] = '/'; lDir[1] = '\0';}

  if (String(lDir).indexOf('/', 1) == -1) 
        onlyRoot = true;
  else  onlyRoot = false;
  
  Dir dir = SPIFFS.openDir(lDir);

  _dThis = false;
  Debugln();
  while (dir.next()) {
    File f = dir.openFile("r");
    if (onlyRoot) {
      if (dir.fileName().indexOf('/', 1) == -1) {
        Debugf("  %-25s %6d \n", dir.fileName().c_str(), f.size());
      }
    } else {
      Debugf("    %-23s %6d \n", dir.fileName().c_str(), f.size());
    }
    yield();
  }
  DebugFlush();

} // listSPIFFS()

//===========================================================================================
void infoSPIFFS() {
//===========================================================================================

  SPIFFS.info(SPIFFSinfo);

  int32_t space = (int32_t)(SPIFFSinfo.totalBytes - SPIFFSinfo.usedBytes);
  Debugln("");
  Debugf("Available SPIFFS space [%8d]bytes\n", freeSpace());
  Debugf("           SPIFFS Size [%8d]kB\n", (SPIFFSinfo.totalBytes / 1024));
  Debugf("     SPIFFS block Size [%8d]bytes\n", SPIFFSinfo.blockSize);
  Debugf("      SPIFFS page Size [%8d]bytes\n", SPIFFSinfo.pageSize);
  Debugf(" SPIFFS max.Open Files [%8d]\n\n", SPIFFSinfo.maxOpenFiles);


} // infoSPIFFS()

//===========================================================================================
String listPulsFiles(String checkedFile) {
//===========================================================================================
  String selHTML = "<select>";
  Dir dir = SPIFFS.openDir("/pls");

  while (dir.next()) {
    File f = dir.openFile("r");
    if (dir.fileName().indexOf(".pls") > -1) {
      selHTML += "<option value='" + dir.fileName() + "'";
      if (dir.fileName().indexOf(checkedFile) > -1) selHTML += " selected ";
      selHTML += ">" + dir.fileName() + "</option>";
    }
    yield();
  }

  selHTML += "</select>";

  return selHTML;

} // listPulsFiles()


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
