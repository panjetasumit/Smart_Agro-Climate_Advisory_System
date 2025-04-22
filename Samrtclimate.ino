#define BLYNK_TEMPLATE_ID "TMPL3irGfGORx"
#define BLYNK_TEMPLATE_NAME "Smart Plant"
#define BLYNK_AUTH_TOKEN "XBQwZ_CIgG9EqLiTIsn4KMW78GdREkC0"

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <BlynkSimpleEsp8266.h>
#include <ArduinoJson.h>

// WiFi credentials
char ssid[] = "OnePlus Nord CE 2";
char pass[] = "12345678";

// Pins
#define DHTPIN D4
#define DHTTYPE DHT11
#define SOIL_PIN A0
#define RELAY_PIN D3
#define PIR_PIN D5

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
BlynkTimer timer;

#define DRY_VALUE 900
#define WET_VALUE 100
#define ADC_REF_VOLTAGE 1.0

int pirEnabled = 0;
int lastMotionState = LOW;
int waterPumpState = 0;

String flaskServer = "http://192.168.29.11:5000/predict";

// BLYNK WRITE FUNCTIONS
BLYNK_WRITE(V6) {
  pirEnabled = param.asInt();
}

BLYNK_WRITE(V12) {
  waterPumpState = param.asInt();
  digitalWrite(RELAY_PIN, waterPumpState == 1 ? LOW : HIGH); // Active LOW
  lcd.setCursor(11, 0);
  lcd.print(waterPumpState == 1 ? "W:ON " : "W:OFF");
}

// Soil moisture %
int getSoilMoisturePercent() {
  int raw = analogRead(SOIL_PIN);
  int percent;
  if (raw >= DRY_VALUE) percent = 0;
  else if (raw <= WET_VALUE) percent = 100;
  else percent = (DRY_VALUE - raw) * 100 / (DRY_VALUE - WET_VALUE);
  return constrain(percent, 0, 100);
}

// Get weather prediction from Flask
void callFlaskAPI() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  float dewpoint = temperature - ((100 - humidity) / 5.0);
  int soil = getSoilMoisturePercent();

  // LCD Line 1
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print((int)temperature);
  lcd.print(" H:");
  lcd.print((int)humidity);
  lcd.print(" ");
  lcd.setCursor(11, 0);
  lcd.print(waterPumpState ? "W:ON " : "W:OFF");

  // LCD Line 2
  lcd.setCursor(0, 1);
  lcd.print("M:");
  lcd.print(soil);
  lcd.print("%      ");

  // Call Flask
  WiFiClient client;
  HTTPClient http;
  String url = flaskServer + "?pressure=1015.9&dewpoint=" + dewpoint +
               "&humidity=" + humidity + "&cloud=81&sunshine=0.0" +
               "&winddirection=40.0&windspeed=13.7";

  http.begin(client, url);
  int httpCode = http.GET();

  if (httpCode == 200) {
    String response = http.getString();
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, response);
    if (!error) {
      const char* rain = doc["rainfall"];
      const char* crop = doc["suggested_crop"];
      Blynk.virtualWrite(V10, rain);
      Blynk.virtualWrite(V11, crop);

      // LCD temporary display
      lcd.setCursor(0, 1);
      lcd.print("Rain: ");
      lcd.print(rain);
      delay(2000);

      lcd.setCursor(0, 1);
      lcd.print("Crop: ");
      lcd.print(crop);
      delay(2000);
    }
  }
  http.end();

  // Send to Blynk
  Blynk.virtualWrite(V0, temperature);
  Blynk.virtualWrite(V1, humidity);
  Blynk.virtualWrite(V3, soil);
}

// PIR Motion
void checkMotion() {
  int motion = digitalRead(PIR_PIN);
  if (pirEnabled) {
    if (motion == HIGH && lastMotionState == LOW) {
      Blynk.virtualWrite(V5, 255);
      Blynk.logEvent("motion_detected", "Motion detected in the field");
      lastMotionState = HIGH;
    } else if (motion == LOW && lastMotionState == HIGH) {
      Blynk.virtualWrite(V5, 0);
      lastMotionState = LOW;
    }
  } else {
    Blynk.virtualWrite(V5, 0);
    lastMotionState = LOW;
  }
}

// Setup
void setup() {
  Serial.begin(9600);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Relay OFF
  pinMode(PIR_PIN, INPUT);

  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");

  dht.begin();

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");
  lcd.clear();
  lcd.print("WiFi Connected");

  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  delay(2000);
  lcd.clear();

  timer.setInterval(1000L, checkMotion);
  timer.setInterval(15000L, callFlaskAPI);
}

// Loop
void loop() {
  Blynk.run();
  timer.run();
}
