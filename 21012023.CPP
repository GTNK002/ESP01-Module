#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>

int gpio_2 = 2;
int hotspotap = 1;

MDNSResponder mdns;

String Domain = "ESP01S001";
String Dpass = "Serial#001";

ESP8266WebServer server(80);

void handleRoot() {
 server.send(200, "text/html", 
"<!DOCTYPE html>" \
"<html>" \
"<head>" \
"<title>home</title>" \
"</head>" \
"<body>" \
"<h1>Home Automation</h1>" \
"</body>" \
"</html>" \
);
}

void factoryreset() {
  if (digitalRead(gpio_2) == LOW) { //GPIO 2
    Serial.printf("GPIO 2 Button Pressed!\n");
    delay(100);
    int startTime = millis();
    while (digitalRead(gpio_2) == LOW) delay(50);
    int endTime = millis();

    if ((endTime - startTime) > 10000) {
      // If key pressed for more than 10secs, reset all
      Serial.printf("Reset to factory.\n");
      Serial.println("clearing eeprom");
      for (int i = 0; i < 96; ++i) {
        EEPROM.write(i, 0);
      }
      EEPROM.commit();
      ESP.reset();
    }
  }
}


void getssidandpassword() {
 server.send(200, "text/html", 
"<!DOCTYPE html>" \
"<html>" \
"<head>" \
"<title>home</title>" \
"</head>" \
"<body>" \
"<h1>Home Automation</h1>" \
"<form method='get' action='setting'>" \
"<p><label>SSID NAME: </label><input name='ssid' length=32></p>" \
"<p><label>PASSWORD: </label><input name='pass' length=64></p>" \
"<p><input type='submit'></form></p>" \
"</body>" \
"</html>" \
);
}

void setting() {
  server.send(200, "text/html", 
  "<head>" \
  "</head>" \
  "<body>" \
  "<h1>Home Automation</h1>" \
  "<p>Please Wait...........</p>" \
  "</body>" \
  "</html>" \
  );
  String qsid = server.arg("ssid");
  String qpass = server.arg("pass");
  delay(1000);
  if (qsid.length() > 0) {
    Serial.println("clearing eeprom");
    for (int i = 0; i < 96; ++i) {
      EEPROM.write(i, 0);
    }
    Serial.println(qsid);
    Serial.println("");
    Serial.println(qpass);
    Serial.println("");
    Serial.println("writing eeprom ssid:");
    for (int i = 0; i < qsid.length(); ++i) {
      EEPROM.write(i, qsid[i]);
      Serial.print("Wrote: ");
      Serial.println(qsid[i]);
    }
    Serial.println("writing eeprom pass:");
    for (int i = 0; i < qpass.length(); ++i) {
      EEPROM.write(32 + i, qpass[i]);
      Serial.print("Wrote: ");
      Serial.println(qpass[i]);
    }
    EEPROM.commit();
    ESP.reset();
  } 
  else {
    Serial.println("Sending 404");
  }
}

void setup(void){
  
  Serial.begin(115200);

  EEPROM.begin(512);

  String esid;
  for (int i = 0; i < 32; ++i)
  {
    esid += char(EEPROM.read(i));
  }
  Serial.println();
  Serial.print("SSID: ");
  Serial.println(esid);
  Serial.println("Reading EEPROM pass");

  String epass;
  for (int i = 32; i < 96; ++i)
  {
    epass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(epass);

  String hotspot_status;
  for (int i = 96; i < 99; ++i)
  {
    hotspot_status += char(EEPROM.read(i));
  }
  Serial.print("hotspot_status: ");
  Serial.println(hotspot_status);

  if (hotspot_status == "ON") {
    WiFi.softAP(Domain, Dpass); // SSID & Password
    Serial.println("softap");
    Serial.println("");
    Serial.println(WiFi.softAPIP());
    server.on("/", handleRoot);     
    server.begin();                 
    Serial.println("HTTP server started");
  }

  else if (esid == "") {
    WiFi.softAP(Domain, Dpass); // SSID & Password
    Serial.println("softap");
    Serial.println("");
    Serial.println(WiFi.softAPIP());
    server.on("/", getssidandpassword);     
    server.on("/setting", setting);  
    server.begin();                 
    Serial.println("HTTP server started");
  }

  else{
    hotspotap = 0;
    WiFi.begin(esid.c_str(), epass.c_str());
    Serial.println("");
    for (int i = 0; i < 12; ++i) {
      Serial.print(".");
      delay(500);
    }
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("wifi connected");
    }
    else {
      Serial.println("wifi not connected!!!");
      delay(1000);
    }

    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(esid.c_str());
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP()); 

    if(mdns.begin(Domain)){
      Serial.println("MDNS Started: " + Domain + ".local/");
    }
  
    server.on("/", handleRoot);

    server.begin();                 
    Serial.println("HTTP server started");
  }
}

void loop(void){
  factoryreset();
  if (WiFi.status() != WL_CONNECTED && hotspotap == 0) {
    Serial.print("Not Connected ");
  }
  mdns.update();
  server.handleClient();  
}
