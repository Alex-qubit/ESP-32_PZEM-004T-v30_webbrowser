#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <Arduino_JSON.h>
#include <PZEM004Tv30.h>
#include <HardwareSerial.h>

#define wifiLed 15

HardwareSerial PZSerial2(2);
PZEM004Tv30 pzem(&PZSerial2);

const char* ssid     = "****"; //your ssid
const char* password = "****"; //your wifi password 

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create an Event Source on /events
AsyncEventSource events("/events");

// Json Variable to Hold Sensor Readings
JSONVar readings;

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 1000;

String getSensorReadings(){
  readings["current"] = String(pzem.current()); //Получение данных о токе и других ниже
  readings["voltage"] =  String(pzem.voltage());
  readings["power"] =  String(pzem.power());
  readings["energy"] = String(pzem.energy());
  String jsonString = JSON.stringify(readings); // Преобразование в json
  return jsonString;
}

// Initialize SPIFFS
void initSPIFFS() {
   if (!SPIFFS.begin()) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}

// Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}
void wifiControl()
{
  if (WiFi.status() == WL_CONNECTED){
    digitalWrite(wifiLed, HIGH);  //Зажигаем светодиод подключения к wifi
  }else {
    digitalWrite(wifiLed, LOW);
  }
}

void setup() {
 Serial.begin(115200);
 pinMode(wifiLed, OUTPUT);
 delay (500);  
 pzem.setAddress(0x42);
 
 initWiFi();
 delay(2000);
 initSPIFFS();
 // Web Server Root URL
 server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
 request->send(SPIFFS, "/index.html", String(), false);
  });
 server.serveStatic("/", SPIFFS, "/");
 
 // Request for the latest sensor readings
  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = getSensorReadings();
    request->send(200, "application/json", json);
    json = String();
  });

  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);

  // Start server
  server.begin();
 // pzem.resetEnergy(); //Раскомментировать для сброса счетчика энергии.
}
void loop() {
  wifiControl();
  if ((millis() - lastTime) > timerDelay) {
    // Send Events to the client with the Sensor Readings Every 10 seconds
    events.send("ping",NULL,millis());
    events.send(getSensorReadings().c_str(),"new_readings" ,millis());
    lastTime = millis();
  }
}
