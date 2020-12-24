#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <TM1637Display.h>

Adafruit_BMP085 bmp;
#define DHTPIN 1  //GPIO1 (Tx)
#define DHTTYPE  DHT11
#define ONE_WIRE_BUS 14 // GPIO14 кастомно запаяный на пин (слева низу)

#define DELAY 30000  //Интервал срабатывания в мс

const char* ssid = "xneek-home"; //wifi ssid
const char* password = "9374490163"; //wifi password
const char* host = "fednik.ru";
const int httpPort = 80;

DHT dht(DHTPIN, DHTTYPE, 15);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
TM1637Display display(2,0);

void setup() {
  pinMode (1, FUNCTION_3);
  pinMode (3, FUNCTION_3);
  Serial.begin(9600);

  // Устанавливаем яркость от 0 до 7
  display.setBrightness(3);
  display.clear();
  
  Wire.pins(2, 0);
  Wire.begin(2, 0);
  
  dht.begin(); // Влажность и температура
  bmp.begin(); // Давление и температура
  sensors.begin();// Термометры ds18b20
  
  sensors.setResolution(10);
  
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
}


int di = 0;

void loop() { 
  int deviceCount = sensors.getDeviceCount();
  sensors.requestTemperatures();
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  
  String url = "/weather/add.php";

  url += "?humidity1=";
  url += String(humidity);
  url += "&humidity2=";
  url += "0";
  url += "&pressure=";
  url += bmp.readPressure();
  url += "&lighting=";
  url += analogRead(6);
  url += "&id_src=";
  url += "1";
  url += "&mac_address=";
  url += WiFi.macAddress();
  url += "&temp1=";
  url += String(temperature);
  url += "&temp2=";
  url += String(bmp.readTemperature());

  for (int i = 0;  i < deviceCount;  i++) {
    float tempC = sensors.getTempCByIndex(i);
    url += "&temp" + String(3 + i) + "=";
    url += String(tempC);
  }

  WiFiClient client;

  if (!client.connect(host, httpPort)) {
    return;
  }
  
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

  if(di == 0) { display.showNumberDec(round(humidity)); }
  if(di == 1) { display.showNumberDec(round((bmp.readPressure()/100) * 0.75)); }
  if(di == 2) { display.showNumberDec(round(temperature)); }
  if(di == 3) { display.showNumberDec(round(sensors.getTempCByIndex(0))); }
  if(di == 4) { display.showNumberDec(round(sensors.getTempCByIndex(1))); }
  if(di == 5) { display.showNumberDec(round(sensors.getTempCByIndex(2))); }
  di++;
  if(di > 5) {
    di = 0;
  }

  delay(DELAY);
}