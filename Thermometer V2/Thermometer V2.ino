//Networking Libraries
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPConnect.h>
#include <ESP8266mDNS.h>
#include <FS.h>

// Libraries for DS18B20
#include <OneWire.h>
#include <DallasTemperature.h>

// Library for Display
#include <U8g2lib.h>
#include <Wire.h>

//Set this to '0' to stop debug messages
#define DEBUG 1

//Set this to '0' to get Farenheit reading
#define CELSIUS 1

//Amount of time, the sensor meausres the body
#define MEASUREMENT_TIME 45000

//softAP credentials
#define SSID "Sumāto ondokei"
#define PASS "Sharingan"

// Data wire is plugged into pin 12 on the esp8266
#define ONE_WIRE_BUS 12

const IPAddress Ip(192, 168, 12, 7);
const IPAddress gateway(192, 168, 12, 7);
const IPAddress subnet(255, 255, 255, 0);

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// U8G2 reference works with Adafruit ESP8266/32u4/ARM Boards + FeatherWing OLED
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);  

unsigned long themometerStartTime = 0;

bool WebSocketConnected = false;
bool deepSleepThemometer = false;
bool StartReadingTemp = false;

void setup() {
  Serial.begin(115200); // Start the Serial for debug
  
  if(!SPIFFS.begin()){
     #if Debug
     Serial.println("An Error has occurred while mounting SPIFFS");
     #endif
     return;
  }

  sensors.begin(); // Inititalize the sensor 
  u8g2.begin(); // Initialize the display
  pinMode(0,INPUT);
  // Bootpage
  drawFrame(firstBoot);

  //Start the hotspot
  WiFi.mode(WIFI_AP);
  WiFi.softAP(SSID, PASS);
  WiFi.softAPConfig(Ip,gateway,subnet);

  //Start server
  initWebSocket();
  if (!MDNS.begin("Temperature")) {             // Start the mDNS responder for temperature.local
    Serial.println("Error setting up MDNS responder!");
  }
  StartServer();
  MDNS.addService("http", "tcp", 80);
}

void loop() {

  MDNS.update();
  ws.cleanupClients();

  if(millis() - themometerStartTime > 60000 && !WebSocketConnected)
  {
    deepSleepMCU();
  }

  delay(2000);
  //ready to measure temperature
  drawFrame(readyToMeasure);
  //wait for the start button to trigger
  if(StartReadingTemp)
  {
    delay(200);
    //show reading temperature 
    drawFrame(ReadingTemperature);

    unsigned long tempStartTime = millis();
    float sum = 0;
    int count = 1;
    //Measure temperature

    #if DEBUG
    Serial.println("Reading temperature");
    #endif

    delay(2000);
    while(millis() - tempStartTime < MEASUREMENT_TIME)
    {
        float temp = getTemperature();
        sum += temp ;
        notifyClients(String(temp));
        delay(2000);
        count++;
    }
    float avg = sum/count;

   //Show temperature
   showTemperature(avg);
   notifyClients(String(avg));
    
    #if DEBUG
    Serial.println("Temperature = " + String(avg));
    #endif

  //Wait for 5 seconds
    delay(5000);
    StartReadingTemp = false;
    notifyClients("stop");
    themometerStartTime = millis();
  }
}

