#include <ETH.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <WiFiGeneric.h>
#include <WiFiMulti.h>
#include <WiFiScan.h>
#include <WiFiServer.h>
#include <WiFiSTA.h>
#include <WiFiType.h>
#include <WiFiUdp.h>

  #ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif

#include <Wire.h>
#include <HardwareSerial.h>

// Replace with your SSID and Password
const char* ssid     = "iPhoneG";
const char* password = "Catrina007";

// Replace with your unique IFTTT URL resource
const char* resource = "/trigger/Sensor_readings/with/key/ipQFYlREwbhhuESK4NV2hkxRwb02V4GkEBp1wLiXng1";\
// How your resource variable should look like, but with your own API KEY (that API KEY below is just an example):
//const char* resource = "/trigger/bme280_readings/with/key/nAZjOphL3d-ZO4N3k64-1A7gTlNSrxMJdmqy3";
// Maker Webhooks IFTTT
const char* server = "maker.ifttt.com";

// Time to sleep
uint64_t uS_TO_S_FACTOR = 1000000;  // Conversion factor for micro seconds to seconds
// sleep for 30 minutes = 1800 seconds
uint64_t TIME_TO_SLEEP = 10;

void setup() {
  {
  Serial.begin(9600);
  Serial1.begin(9600,SERIAL_8N1, 4, 2);    //Baud rate, parity mode, RX, TX
  Serial2.begin(9600,SERIAL_8N1, 16, 17);
  Serial.println("Serial Txd is on pin: "+String(2));
  Serial.println("Serial Rxd is on pin: "+String(4));
  Serial.println("Serial Txd is on pin: "+String(17));
  Serial.println("Serial Rxd is on pin: "+String(16));
}

  Serial.begin(9600); 
  delay(2000);
  initWifi();
  makeIFTTTRequest();
    
  #ifdef ESP32
    // enable timer deep sleep
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);    
    Serial.println("Going to sleep now");
    // start deep sleep for 3600 seconds (60 minutes)
    esp_deep_sleep_start();
  #else
    // Deep sleep mode for 3600 seconds (60 minutes)
    Serial.println("Going to sleep now");
    ESP.deepSleep(TIME_TO_SLEEP * uS_TO_S_FACTOR); 
  #endif
}


void loop() {
  // sleeping so wont get here 
}

// Establish a Wi-Fi connection with your router
void initWifi() {
  Serial.print("Connecting to: "); 
  Serial.print(ssid);
  WiFi.begin(ssid, password);  

  int timeout = 10 * 4; // 10 seconds
  while(WiFi.status() != WL_CONNECTED  && (timeout-- > 0)) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("");

  if(WiFi.status() != WL_CONNECTED) {
     Serial.println("Failed to connect, going back to sleep");
  }

  Serial.print("WiFi connected in: "); 
  Serial.print(millis());
  Serial.print(", IP address: "); 
  Serial.println(WiFi.localIP());
}

// Make an HTTP request to the IFTTT web service
void makeIFTTTRequest() {
  Serial.print("Connecting to "); 
  Serial.print(server);
  
  WiFiClient client;
  int retries = 5;
  while(!!!client.connect(server, 80) && (retries-- > 0)) {
    Serial.print(".");
  }
  Serial.println();
  if(!!!client.connected()) {
    Serial.println("Failed to connect...");
  }
  
  Serial.print("Request resource: "); 
  Serial.println(resource);

String storedData = "";
 if( Serial.available() ){ // if new data is coming from the HW Serial
   while(Serial.available())          // reading data into char array
   {
     char inChar = Serial.read();
     storedData += inChar;
// This makes a string named storedData
   }
 }
 String storedData2 = "";
 if( Serial.available() ){ // if new data is coming from the HW Serial
   while(Serial.available())          // reading data into char array
   {
     char inChar = Serial.read();
     storedData2 += inChar;
// This makes a string named storedData
   }
 }

 String jsonObject = String("{\"value1\":\"") + storedData + "\",\"value2\":\"" + storedData2 + "\"}";

 client.println(String("POST ") + resource + " HTTP/1.1");
  client.println(String("Host: ") + server); 
  client.println("Connection: close\r\nContent-Type: application/json");
  client.print("Content-Length: ");
  client.println(jsonObject.length());
  client.println();
  client.println(jsonObject);
        
  int timeout = 5 * 10; // 5 seconds             
  while(!!!client.available() && (timeout-- > 0)){
    delay(100);
  }
  if(!!!client.available()) {
    Serial.println("No response...");
  }
  while(client.available()){
    Serial.write(client.read());
  }
  
  Serial.println("\nclosing connection");
  client.stop(); 
}
