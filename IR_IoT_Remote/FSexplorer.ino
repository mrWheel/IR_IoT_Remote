/*
***************************************************************************  
**  Program  : FSexplorer, part of IR_IoT_Remote
**  Version  : v0.1.0 (WS)
**
**  Mostly stolen from https://www.arduinoforum.de/User-Fips
**  See also https://www.arduinoforum.de/arduino-Thread-SPIFFS-DOWNLOAD-UPLOAD-DELETE-Esp8266-NodeMCU
**
***************************************************************************      
*/

File fsUploadFile;                      // Stores de actuele upload

void handleRoot() {                     // HTML FSexplorer
  FSInfo fs_info;
  SPIFFS.info(fs_info);
  String FSexplorerHTML;
  FSexplorerHTML += "<!DOCTYPE HTML><html lang='en-US'>";
  FSexplorerHTML += "<head>";
  FSexplorerHTML += "<meta charset='UTF-8'>";
  FSexplorerHTML += "<meta name= viewport content='width=device-width, initial-scale=1.0,' user-scalable=yes>";
  FSexplorerHTML += "<style type='text/css'>";
  FSexplorerHTML += "body {background-color: lightblue;}";
  FSexplorerHTML += "</style>";
  FSexplorerHTML += "</head>";
  FSexplorerHTML += "<body><h1>IR_IoT_Remote FSexplorer</h1><h2>Upload, Download of Verwijder</h2>";

  FSexplorerHTML += "<hr><h3>Selecteer bestand om te downloaden:</h3>";
  if (!SPIFFS.begin())  { _dThis = true; Debugln("SPIFFS failed to mount !\n"); }

  Dir dir = SPIFFS.openDir("/");         // List files on SPIFFS
  while (dir.next())  {
    FSexplorerHTML += "<a href ='";
    FSexplorerHTML += dir.fileName();
    FSexplorerHTML += "?download='>";
    FSexplorerHTML += "SPIFFS";
    FSexplorerHTML += dir.fileName();
    FSexplorerHTML += "</a> ";
    FSexplorerHTML += formatBytes(dir.fileSize()).c_str();
    FSexplorerHTML += "<br>";
  }

  FSexplorerHTML += "<p><hr><big>Sleep bestand om te verwijderen:</big>";
  FSexplorerHTML += "<form action='/FSexplorer' method='POST'>Om te verwijderen ";
  FSexplorerHTML += "<input type='text' style='height:45px; font-size:15px;' name='Delete' placeholder=' Bestand hier in-slepen ' required>";
  FSexplorerHTML += "<input type='submit' class='button' name='SUBMIT' value='Verwijderen'>";
  FSexplorerHTML += "</form><p><br>";
  
  FSexplorerHTML += "<hr>";
  FSexplorerHTML += "<form method='POST' action='/FSexplorer/upload' enctype='multipart/form-data' style='height:35px;'>";
  FSexplorerHTML += "<big>Bestand uploaden: &nbsp;</big>";
  FSexplorerHTML += "<input type='file' name='upload' style='height:35px; font-size:14px;' required>";
  FSexplorerHTML += "<input type='submit' value='Upload' class='button'>";
  FSexplorerHTML += "</form>";
  
  FSexplorerHTML += "<form style='float: left;' action='/update' method='GET'><big>Update Firmware </big>";
  FSexplorerHTML += "  <input type='submit' class='button' name='SUBMIT' value='select Firmware'>";
  FSexplorerHTML += "</form>";
  FSexplorerHTML += "<br>";
  
  FSexplorerHTML += "<hr>Omvang SPIFFS: ";
  FSexplorerHTML += formatBytes(fs_info.totalBytes).c_str();      
  FSexplorerHTML += "<br>Waarvan in gebruik: ";
  FSexplorerHTML += formatBytes(fs_info.usedBytes).c_str();      
  FSexplorerHTML += "<p>";

  FSexplorerHTML += "<hr>";
  FSexplorerHTML += "<div style='width: 30%'>";
  FSexplorerHTML += "  <form style='float: left;' action='/ReBoot' method='POST'>ReBoot IR_IoT_Remote ";
  FSexplorerHTML += "    <input type='submit' class='button' name='SUBMIT' value='ReBoot'>";
  FSexplorerHTML += "  </form>";

  FSexplorerHTML += "  <form style='float: right;' action='/' method='POST'> &nbsp; Exit FSexplorer ";
  FSexplorerHTML += "   <input type='submit' class='button' name='SUBMIT' value='Exit'>";
  FSexplorerHTML += "  </form>";
  FSexplorerHTML += "</div>";
  FSexplorerHTML += "<div style='width: 80%'>&nbsp;</div>";
  
  FSexplorerHTML += "</body></html>\n";

  HttpServer.send(200, "text/html", FSexplorerHTML);
}

String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + " Byte";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + " KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + " MB";
  }
}

String getContentType(String filename) {
  if (HttpServer.hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";

} // getContentType()


void handleReBoot() {
  String redirectHTML = "";

  redirectHTML += "<!DOCTYPE HTML><html lang='en-US'>";
  redirectHTML += "<head>";
  redirectHTML += "<meta charset='UTF-8'>";
  redirectHTML += "<style type='text/css'>";
  redirectHTML += "body {background-color: lightblue;}";
  redirectHTML += "</style>";
  redirectHTML += "<title>Redirect to IR_IoT_Remote</title>";
  redirectHTML += "</head>";
  redirectHTML += "<body><h1>IR_IoT_Remote Onderhoud</h1>";
  redirectHTML += "<h3>Rebooting IR_IoT_Remote</h3>";
  redirectHTML += "<br><div style='width: 500px; position: relative; font-size: 25px;'>";
  redirectHTML += "  <div style='float: left;'>Redirect over &nbsp;</div>";
  redirectHTML += "  <div style='float: left;' id='counter'>20</div>";
  redirectHTML += "  <div style='float: left;'>&nbsp; seconden ...</div>";
  redirectHTML += "  <div style='float: right;'>&nbsp;</div>";
  redirectHTML += "</div>";
  redirectHTML += "<!-- Note: don't tell people to `click` the link, just tell them that it is a link. -->";
  redirectHTML += "<br><br><hr>If you are not redirected automatically, click this <a href='/'>IR_IoT_Remote</a>.";
  redirectHTML += "  <script>";
  redirectHTML += "      setInterval(function() {";
  redirectHTML += "          var div = document.querySelector('#counter');";
  redirectHTML += "          var count = div.textContent * 1 - 1;";
  redirectHTML += "          div.textContent = count;";
  redirectHTML += "          if (count <= 0) {";
  redirectHTML += "              window.location.replace('/'); ";
  redirectHTML += "          } ";
  redirectHTML += "      }, 1000); ";
  redirectHTML += "  </script> ";
  redirectHTML += "</body></html>\n";
  
  HttpServer.send(200, "text/html", redirectHTML);
  
  TelnetStream.println("ReBoot IR_IoT_Remote ..");
  TelnetStream.flush();
  delay(1000);
  ESP.reset();
  
} // handleReBoot()


bool handleFileRead(String path) {
  _dThis = true;
  Debugln("handleFileRead: " + path);
  if (path.endsWith("/")) path += "aircoCntrlr.html";
  String contentType = getContentType(path);
  Debugln(contentType);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = HttpServer.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
  
} // handleFileRead()


void handleFileDelete() {                               
  String file2Delete, hostNameURL, IPaddressURL;                     
  if (HttpServer.args() == 0) return handleRoot();
  if (HttpServer.hasArg("Delete")) {
    file2Delete = HttpServer.arg("Delete");
    file2Delete.toLowerCase();
    Dir dir = SPIFFS.openDir("/");
    while (dir.next())    {
      String path = dir.fileName();
      path.replace(" ", "%20"); path.replace("ä", "%C3%A4"); path.replace("Ä", "%C3%84"); path.replace("ö", "%C3%B6"); path.replace("Ö", "%C3%96");
      path.replace("ü", "%C3%BC"); path.replace("Ü", "%C3%9C"); path.replace("ß", "%C3%9F"); path.replace("€", "%E2%82%AC");
      hostNameURL   = "http://" + String(_HOSTNAME) + ".local" + path + "?download=";
      hostNameURL.toLowerCase();
      IPaddressURL  = "http://" + WiFi.localIP().toString() + path + "?download=";
      IPaddressURL.toLowerCase();
    //if (HttpServer.arg("Delete") != "http://" + WiFi.localIP().toString() + path + "?download=" )
      if ( (file2Delete != hostNameURL ) && (file2Delete != IPaddressURL ) ) {
        continue;
      }
      SPIFFS.remove(dir.fileName());
      String header = "HTTP/1.1 303 OK\nLocation:";
      header += HttpServer.uri();
      header += "\nCache-Control: no-cache\n\n";
      HttpServer.sendContent(header);
      return;
    }
    String FSexplorerHTML;                                    
    FSexplorerHTML += "<!DOCTYPE HTML><html lang='de'><head><meta charset='UTF-8'><meta name= viewport content=width=device-width, initial-scale=1.0, user-scalable=yes>";
    FSexplorerHTML += "<meta http-equiv='refresh' content='3; URL=";
    FSexplorerHTML += HttpServer.uri();
    FSexplorerHTML += "'><style>body {background-color: powderblue;}</style></head>\n<body><center><h2>Bestand niet gevonden</h2>wacht 3 seconden...</center>";
    HttpServer.send(200, "text/html", FSexplorerHTML );
  }

} // handleFileDelete()


void handleFileUpload() {                                 
  if (HttpServer.uri() != "/FSexplorer/upload") return;
  HTTPUpload& upload = HttpServer.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    TelnetStream.print("handleFileUpload Name: "); TelnetStream.println(filename);
    if (filename.length() > 30) {
      int x = filename.length() - 30;
      filename = filename.substring(x, 30 + x);
    }
    if (!filename.startsWith("/")) filename = "/" + filename;
    TelnetStream.print("handleFileUpload Name: "); TelnetStream.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    TelnetStream.print("handleFileUpload Data: "); TelnetStream.println(upload.currentSize);
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile)
      fsUploadFile.close();
    yield();
    TelnetStream.print("handleFileUpload Size: "); TelnetStream.println(upload.totalSize);
    handleRoot();
  }
  
} // handleFileUpload()


//===========================================================================================
void reloadPage(String goTo) {
//===========================================================================================
    String goToPageHTML;                                    
    goToPageHTML += "<!DOCTYPE HTML><html lang='de'><head><meta charset='UTF-8'>";
    goToPageHTML += "<meta name= viewport content=width=device-width, initial-scale=1.0, user-scalable=yes>";
  //goToPageHTML += "<meta http-equiv='refresh' content='1; URL=" + goTo + "'>";
    goToPageHTML += "<style>body {background-color: powderblue;}</style>";
    goToPageHTML += "</head>\r\n<body><center>Wait ..</center>";
    goToPageHTML += "<br><br><hr>If you are not redirected automatically, click this <a href='/'>"+String(_HOSTNAME)+"</a>.";
    goToPageHTML += "  <script>";
    goToPageHTML += "    window.location.replace('" + goTo + "'); ";
    goToPageHTML += "  </script> ";
    HttpServer.send(200, "text/html", goToPageHTML );
  
} // reloadPage()

//void formatSpiffs() {       // Format SPIFFS
//  SPIFFS.format();
//  handleRoot();
//}
