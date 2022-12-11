#define BLYNK_TEMPLATE_ID "TMPLfJQ89Pug"
#define BLYNK_DEVICE_NAME "Plant watering system"
#define BLYNK_AUTH_TOKEN "VftUNwKEgm9a9TATviEzowq0iP7ZYx5Y"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include<DHT.h>
#include<ThingSpeak.h>
#include"HTTPClient.h"
#include"time.h"
#define CSE_IP "192.168.43.205"
#define CSE_PORT 5089
#define OM2M_ORGIN "admin:admin"
#define OM2M_MN "/~/in-cse/in-name/"
#define OM2M_AE "Plant_Health_Monitoring_Smart_Watering"
#define OM2M_DATA_CONT "Node-1/Data"
HTTPClient http;
WiFiClient client;

unsigned long myChannel_number=1976846;
const char * myWriteAPIKEY= "LLO4LCVCP33KFC1K";
#define DHTPIN 13
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);
int sm_sensor=34;

const char * ntpServer = "pool.ntp.org";

long int prev_millis = 0;
unsigned long ntime;

unsigned long getTime() 
{ 
  time_t now;
  struct tm timeinfo;
    if (!getLocalTime( & timeinfo)) {
      Serial.println("Failed to obtain time");
      return (0);
    }
    time( & now);
    return now;
}


 BLYNK_WRITE(V3)
{
  int pinvalue = param.asInt();
  digitalWrite(15,pinvalue);
}  

char auth[] = BLYNK_AUTH_TOKEN;

char ssid[] = "qwerty";  // type your wifi name
char pass[] = "12345678";  // type your wifi password

BlynkTimer timer;

void sendSensor()
{
  //Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  int moisture_sensor= analogRead(sm_sensor);
  // Check if any reads failed and exit early (to try again).
   if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
    Blynk.virtualWrite(V0, t);
    Blynk.virtualWrite(V1, h);
    Blynk.virtualWrite(V2, moisture_sensor);
    delay(500);
}
void setup()
{   
  configTime(0, 0, ntpServer);
   Serial.begin(9600);
   pinMode (15,OUTPUT);
  Blynk.begin(auth, ssid, pass);
  ThingSpeak.begin(client);
  timer.setInterval(100L, sendSensor);
 
  }

void loop()
{
  Blynk.run();
   timer.run();
  
   float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  int moisture_sensor= analogRead(sm_sensor);
  
  ThingSpeak.setField(1,t);
  ThingSpeak.setField(2,h);
  ThingSpeak.setField(3,moisture_sensor);
  ThingSpeak.writeFields(myChannel_number, myWriteAPIKEY);
  
  if (millis() - prev_millis >= 2000) 
  {
   ntime = getTime();
    String data;
    String server = "http://" + String() + CSE_IP + ":" + String() + CSE_PORT + String() + OM2M_MN;
    data = "[" + String(ntime) + ", " + String(h) + ", " + String(t)+ "]";
    Serial.println(data);
     
    http.begin(server + String() + OM2M_AE + "/" + OM2M_DATA_CONT + "/");

    http.addHeader("X-M2M-Origin", OM2M_ORGIN);
    http.addHeader("Content-Type", "application/json;ty=4");
    http.addHeader("Content-Length", "100");
    
  String req_data = String() + "{\"m2m:cin\": {"
      +
      "\"lbl\": \"" + "Esp32 humidity and Temperature" + "\","
      +
      "\"con\": \"" + data + "\","
      +
      "\"cnf\":\"text\""
      +
      "}}";

   int code = http.POST(req_data);
   http.end();
   Serial.println(code);
   prev_millis = millis();
 }

  delay(5000);
}
