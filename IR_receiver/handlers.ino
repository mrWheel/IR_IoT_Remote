

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


//=======================================================================
String getFileName(String prompt) {
  String fileName = "", answer;

  digitalWrite(GRN_LEDPIN, LOW);
  //-- empty Serial buffer -------
  while (Serial.available() > 0) {
    yield();
    (char)Serial.read();
  }

  Serial.print(prompt + ": ");
  Serial.setTimeout(25000);
  while (fileName.length() <= 0) {
    yield();
    fileName = Serial.readStringUntil('\n');
    fileName.replace("\n", "");
    fileName.replace("\r", "");
    fileName.replace("/pls/", "");
    fileName.replace(".pls", "");
    fileName.replace(" ", "");
    fileName.replace("/", "");
    for (int16_t c = 0; c < fileName.length(); c++) {
      if (   (fileName[c] >= '0' && fileName[c] <= '9') 
          || (fileName[c] >= 'A' && fileName[c] <= 'Z')
          || (fileName[c] >= 'a' && fileName[c] <= 'z')
          ) { 
        //Serial.print((char)fileName[c]); 
      } else {
        fileName[c] = '*';
        //Serial.print((char)fileName[c]); 
      }
    }
  } // while fileName empty
  
  fileName = "/pls/" + fileName + ".pls";
  //prompt = prompt + " [" + fileName + "] (y/N) "; 
  Serial.print("\n" + prompt + " [" + fileName + "] (y/N) ");
  Serial.setTimeout(25000);
  answer = Serial.readStringUntil('\n'); 

  while (Serial.available() > 0) {
    yield();
    inChar = (char)Serial.read();
    while (Serial.available() > 0) {
       yield();
       (char)Serial.read();
    }
  } // Serial.available()

  if (answer[0] == 'y' || answer[0] == 'Y') {
    //Serial.print("\nfileName is [");
    //Serial.print(fileName);
    //Serial.println("]!");
    return String(fileName);
  } 

  return "";
  
} // getFileName()


//=======================================================================
void writeRawData(String fName, const decode_results *results) {

  Serial.printf("writeRawData(%s): aantal pulsen [%ld]\n", fName.c_str(), (results->rawlen - 1));
  if (results->rawlen <= 1) {
     Serial.println("Nothing to write ..");
     return;
  }

  File file = SPIFFS.open(fName, "w");
  uint16_t  pulsNr = 0;
    
  for (uint16_t i = 1; i < results->rawlen; i++) {
    uint32_t usecs;
    yield();
    usecs = results->rawbuf[i] * RAWTICK;
    file.print(usecs);
    file.print(";");
    if (pulsNr >= (results->rawlen - 1))
         Serial.printf("%ld ", usecs);
    else Serial.printf("%ld, ", usecs);
    if (i%20 == 0) Serial.println(" ");
    pulsNr++;
  }

  file.close();  
  Serial.printf("\nwriteRawData(): saved [%ld] pulses\n", pulsNr);

} // writeRawData()


//=======================================================================
void delSPIFFSfile(String fName) {

  Serial.printf("delSPIFFSfile(%s) .. ", fName.c_str());

  SPIFFS.remove(fName.c_str());
  Serial.println(" .. removed\n");

} // delSPIFFSfile()

//=======================================================================
void handleKeyInput() {
  String fileName;
  
  digitalWrite(GRN_LEDPIN, LOW);
  
  while (Serial.available() > 0) {
    yield();
    inChar = (char)Serial.read();
    while (Serial.available() > 0) {
       yield();
       (char)Serial.read();
    }

    switch(inChar) {
      case 'D':     Serial.println("delete pulsefile from SPIFFS");
                    fileName = getFileName("delete pulsfile");
                    Serial.printf("\nfileName is [%s]\n", fileName.c_str());
                    delSPIFFSfile(fileName);
                    break;
      case 'l':
      case 'L':     listSPIFFS();
                    break;
                    
      case 'R':     Serial.println("Reboot ESP8266");
                    ESP.restart();
                    break;
      case 'w':
      case 'W':     Serial.println("Write rawData to SPIFFS");
                    fileName = getFileName("save pulses as");
                    Serial.printf("\nfileName is [%s]\n", fileName.c_str());
                    writeRawData(fileName,  &results);
                    break;
                    
      case 's':
      case 'S':     Serial.println("showStatus()");
                    Serial.println();
                    Serial.print("IRrecvDumpV2 is now running and waiting for IR input on Pin ");
                    Serial.println(kRecvPin);
                    break;
                    
      case 'h':
      case 'H':     
      default:      Serial.println("\nCommando's zijn:\n");
                    Serial.println("  *D - remove file from SPIFFS");
                    Serial.println("   L - List SPIFFS");
                    Serial.println("   H - Help (this message)");
                    Serial.println("   W - Write rawData to SPIFFS");
                    Serial.println("   S - System Status");
                    Serial.println("  *R - Reboot");
                    Serial.println(" ");

    } // switch()
  }
  
} // handleKeyInput()
