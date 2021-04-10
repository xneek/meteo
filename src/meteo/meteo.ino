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

#define DELAY 5000  //Интервал срабатывания в мс

const char* ssids[] = {
  "Galaxy A20s0141",
  "Keenetic-3266",
  "xneek-oneplus"
};

const char* passwords[] = {
  "wndp3042",
  "Qkfr2Ct6",
  "9374490163"
};

const int ssidsCount = sizeof(ssids) / sizeof(char*);

const char* host = "fednik.ru";
const int httpPort = 80;

DHT dht(DHTPIN, DHTTYPE, 15);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
TM1637Display display(2,0);


int currentSsidIndex = 0;

void setup() {
  pinMode (1, FUNCTION_3);
  pinMode (3, FUNCTION_3);
  Serial.begin(9600);

  // Устанавливаем яркость от 0 до 7
  display.setBrightness(2);
  display.clear();
  
  Wire.pins(2, 0);
  Wire.begin(2, 0);
  
  dht.begin(); // Влажность и температура
  bmp.begin(); // Давление и температура
  sensors.begin();// Термометры ds18b20
  
  sensors.setResolution(10);

  while (currentSsidIndex < ssidsCount) {
    delay(10);
    int connectionTry = 20;
    WiFi.begin(ssids[currentSsidIndex], passwords[currentSsidIndex]);
    while (WiFi.status() != WL_CONNECTED && connectionTry > 0)
    {
      delay(500);
      Serial.print(".");
      connectionTry = connectionTry - 1;
      display.showNumberDec(((currentSsidIndex + 1) * 100) + connectionTry);
    }

    currentSsidIndex++;
  }
}


int di = 1;
const int staticDevicesCount = 3; 

void loop() { 
  int deviceCount = sensors.getDeviceCount();
  sensors.requestTemperatures();
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if(di == 1) {
    const uint8_t seg[] = { SEG_E | SEG_F | SEG_G | SEG_B | SEG_C, 0x00, 0x00, 0x00};
    display.clear();
    display.setSegments(seg);
    display.showNumberDec(round(humidity), false, 3, 1);
  }
  if(di == 2) {
    const uint8_t seg[] = {SEG_A | SEG_E | SEG_F | SEG_G | SEG_B, 0x00, 0x00, 0x00};
    display.clear();
    display.setSegments(seg);
    display.showNumberDec(round((bmp.readPressure()/100) * 0.75), false, 3, 1);
  }
  if(di == 3) {
    const uint8_t seg[] = { display.encodeDigit(1), 0x00, 0x00, 0x00 };
    display.clear();
    display.setSegments(seg);
    display.showNumberDec(round(temperature), false, 3, 1);
  }

  if(di > staticDevicesCount && di <= staticDevicesCount + deviceCount) {
    const uint8_t seg[] = { display.encodeDigit(di - (staticDevicesCount - 1)), 0x00, 0x00, 0x00 };
    display.clear();
    display.setSegments(seg);
    display.showNumberDec(round(sensors.getTempCByIndex( di - (staticDevicesCount + 1) )), false, 3, 1);
  }
  
  if (di >= staticDevicesCount + deviceCount + 1) {
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
    if (client.connect(host, httpPort)) {
    
      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                "Host: " + host + "\r\n" +
                "Connection: close\r\n\r\n");
      
      const uint8_t SEG_DONE[] = {
        SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
        SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // Oo
        SEG_C | SEG_E | SEG_G,                           // n
        SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
      };

      display.clear();
      display.setSegments(SEG_DONE);

    } else {
      const uint8_t seg[] = { SEG_A | SEG_G | SEG_D | SEG_E | SEG_F, SEG_E | SEG_G,  SEG_E | SEG_G, 0x00 };
      display.clear();
      display.setSegments(seg);
    }

    di = 0;

  }

  di++;
  delay(DELAY);
}
