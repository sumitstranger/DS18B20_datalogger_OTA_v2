#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <OneWire.h>
#include <HTTPClient.h>
HTTPClient ask;
int c = 0;
int counter = 0;
const unsigned int writeInterval = 30000;   // write interval (in ms)

// ASKSENSORS API host config
const char* host = "api.asksensors.com";  // API host name
const int httpPort = 80;      // port



//28 4F CE 79 97 6 3 EE
// Based on the OneWire library example
#include <DallasTemperature.h>
#include "index.h" //Our HTML webpage contents with javascripts
 #define ONE_WIRE_BUS 33

 
OneWire oneWire(ONE_WIRE_BUS);//SSID and Password of your WiFi router
const char* ssid = "Redmi 5";
const char* password = "asdfghjkl";

DallasTemperature sensors(&oneWire);
DeviceAddress sensor1 = { 0x28, 0x4F, 0xCE, 0x79, 0x97, 0x6, 0x3, 0xEE };

 
WebServer server(80); //Server on port 80
 

void handleRoot() {
 String s = MAIN_page; //Read HTML contents
 server.send(200, "text/html", s); //Send web page
}
 
void handleADC() {
sensors.requestTemperatures();
   float x = sensors.getTempC(sensor1);
   String adcValue = String(x);
   //Serial.println(x);
   server.send(200, "text/plane", adcValue); //Send ADC value only to client ajax request
   send_to_cloud(x);
   c++;
   Serial.println(c);
}


void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  //Increment boot number and print it every reboot


  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(ssid, password);
  
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    counter++;
    if(counter>=20){ //30 seconds timeout - reset board
    ESP.restart();
  }
  }


  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
     server.on("/", handleRoot);      //Which routine to handle at root location. This is display page
  server.on("/readADC", handleADC); //This page is called by java Script AJAX
 
  server.begin();                  //Start server
  Serial.println("HTTP server started");

  

}

void loop() {
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    counter++;
    if(counter>=5 ){ //30 seconds timeout - reset board
    counter = 0 ;
    ESP.restart();
    
  }
  }
  server.handleClient();
 ArduinoOTA.handle();
  
}

void send_to_cloud(float temp)
{
WiFiClient client;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }else {

    // Create a URL for updating module1 and module 2
  String url = "http://api.asksensors.com/write/";
  url += "dUhsf09CAD4E8lLNQ3bQBPwdTZWoznkq";
  url += "?module1=";
  url += temp;

   ask.begin(url); //Specify the URL
  
    //Check for the returning code
    int httpCode = ask.GET();          
 
    if (httpCode > 0) { 
 
        String payload = ask.getString();
  
      } else {
      Serial.println("Error on HTTP request");
    }
 
    ask.end(); 

  }

  client.stop();  // stop client
}
