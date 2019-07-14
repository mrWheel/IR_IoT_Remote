

//=======================================================================
void listSPIFFS() {
  String str = "\n";
  Dir dir = SPIFFS.openDir("/pls");
  while (dir.next()) {
    str += dir.fileName();
    str += " / ";
    str += dir.fileSize();
    str += "\r\n";
  }
  Serial.println(str);
  
} // listSPIFFS()

//===========================================================================================
void listPlsFiles() {
//===========================================================================================
  int8_t nr = 0;

  Dir dir = SPIFFS.openDir("/");

  while (dir.next()) {
    File f = dir.openFile("r");
    if (dir.fileName().indexOf(".pls") > -1) {
      Serial.printf("[%02d]\t[%s]\n", ++nr, dir.fileName().c_str());
    }
    yield();
  }

} // listPlsFiles()


//=======================================================================
String getFileNumber(String prompt) {
  int8_t nr, fileNumber, answer;

  digitalWrite(RED_LEDPIN, LOW);
  //-- empty Serial buffer -------
  while (Serial.available() > 0) {
    yield();
    (char)Serial.read();
  }

  Serial.setTimeout(25000);
  Serial.print("\n" + prompt + ": ");
  yield();
  fileNumber = Serial.parseInt();

  Dir dir = SPIFFS.openDir("/");
  nr = 0;
  while (dir.next()) {
    File f = dir.openFile("r");
    if (dir.fileName().indexOf(".pls") > -1) {
      if (++nr == fileNumber) {
        //Serial.printf("[%02d]\t[%s]\n", nr, dir.fileName().c_str());
        return String(dir.fileName());
      }
    }
    yield();
  }
  
  while (Serial.available() > 0) {
    yield();
    inChar = (char)Serial.read();
    while (Serial.available() > 0) {
       yield();
       (char)Serial.read();
    }
  } // Serial.available()

  return "";
  
} // getFileNumber()


//=======================================================================
String getFileName(int fileNumber) {
  int8_t nr;
  
  Serial.printf("Match name by number[%d]\n", fileNumber);
  
  Dir dir = SPIFFS.openDir("/");
  nr = 0;
  while (dir.next()) {
    File f = dir.openFile("r");
    if (dir.fileName().indexOf(".pls") > -1) {
      if (++nr == fileNumber) {
        //Serial.printf("[%02d]\t[%s]\n", nr, dir.fileName().c_str());
        return String(dir.fileName());
      }
    }
    yield();
  }
  
  while (Serial.available() > 0) {
    yield();
    inChar = (char)Serial.read();
    while (Serial.available() > 0) {
       yield();
       (char)Serial.read();
    }
  } // Serial.available()

  return "";
  
} // getFileName()


//=======================================================================
uint16_t readRawData(String fName, uint16_t inData[]) {
//=======================================================================

  int16_t  pulsNr = 0;

  Serial.printf("readRawData(%s): \n", fName.c_str());
  
  memset(inData, 0, sizeof(inData));
  File file = SPIFFS.open(fName, "r");
    
  while(file.available()) {
    yield();
    inData[pulsNr] = (uint16_t)file.readStringUntil(';').toInt();
    //Debugf("[%ld]%ld, ", pulsNr,inData[pulsNr]);
    //if (pulsNr%20 == 0) Debugln(" ");
    pulsNr++;
  }
  
  file.close();  
  Serial.printf("readRawData(): read [%ld] pulses\n", pulsNr);
  
  return pulsNr;

} // readRawData()


//=======================================================================
void delSPIFFSfile(String fName) {

  Serial.printf("delSPIFFSfile(%s) .. ", fName.c_str());

  SPIFFS.remove(fName.c_str());
  Serial.println(" .. removed\n");

} // delSPIFFSfile()

//=======================================================================
void handleKeyInput() {
  String fileName;
  
  digitalWrite(RED_LEDPIN, LOW);
  
  while (Serial.available() > 0) {
    yield();
    inChar = (char)Serial.read();
    while (Serial.available() > 0) {
       yield();
       (char)Serial.read();
    }

    switch(inChar) {
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':     fileName = getFileName((int)(inChar - '0'));
                    if (fileName.length() < 1) break;
                    Serial.printf("\nfileName is [%s]\n", fileName.c_str());
                    noPulses = readRawData(fileName,  rawData);
                    break;

      case 'D':     Serial.println("delete pulsefile from SPIFFS");
                    fileName = getFileNumber("delete pulsfile");
                    Serial.printf("\nfileName is [%s]\n", fileName.c_str());
                    delSPIFFSfile(fileName);
                    break;
      case 'l':
      case 'L':     //listSPIFFS();
                    listPlsFiles();
                    break;
                    
      case 'R':     Serial.println("Reboot ESP8266");
                    ESP.restart();
                    break;
      case 't':
      case 'T':     Serial.println("Transmit rawData from SPIFFS");
                    fileName = getFileNumber("Pulse file number");
                    if (fileName.length() < 1) break;
                    Serial.printf("\nfileName is [%s]\n", fileName.c_str());
                    noPulses = readRawData(fileName,  rawData);
                    break;
                    
      case 's':
//    case 'S':     Serial.println("showStatus()");
//                  Serial.println();
//                  Serial.print("IRrecvDumpV2 is now running and waiting for IR input on Pin ");
//                  Serial.println(kRecvPin);
//                  break;
                    
      case 'h':
      case 'H':     
      default:      Serial.println("\nCommando's zijn:\n");
                    Serial.println("  *D - remove file from SPIFFS");
                    Serial.println("   L - List SPIFFS");
                    Serial.println("   H - Help (this message)");
                    Serial.println("   T - Transmit rawData from SPIFFS");
//                  Serial.println("   S - System Status");
                    Serial.println("  *R - Reboot");
                    Serial.println(" ");

    } // switch()
  }
  
} // handleKeyInput()
