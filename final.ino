#include <TinyGPS++.h>
#include <WiFi.h>

#include <Arduino.h>
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"


#define WIFI_SSID "test1"
#define WIFI_PASSWORD "test12345678"

#define API_KEY "AIzaSyB_DC-JTuTvhNlqEUiMEnIRtuwJhaMCHKY"
#define DATABASE_URL "https://smartcollarpettracker-68bb8-default-rtdb.firebaseio.com/"

#define GPS_BAUDRATE 9600  // The default baudrate of NEO-6M is 9600

TinyGPSPlus gps;  // the TinyGPS++ object
FirebaseData fbdo;

FirebaseConfig config;
FirebaseAuth auth;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

void setup() {
  Serial.begin(9600);
  Serial2.begin(GPS_BAUDRATE);
  Serial.println(F("ESP32 - GPS module"));
  delay(2000);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback; 

  Firebase.begin(&config, &auth); // Pass the objects to the begin function
  Firebase.reconnectWiFi(true);
}

void loop() {
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    if (Serial2.available() > 0) {
      if (gps.encode(Serial2.read())) {
        if (gps.location.isValid()) {
          Firebase.RTDB.setFloat(&fbdo, "ESP/Latitude", gps.location.lat());
          Serial.print(F("- latitude: "));
          Serial.println(gps.location.lat());

          Firebase.RTDB.setFloat(&fbdo, "ESP/Latitude", gps.location.lng());
          Serial.print(F("- longitude: "));
          Serial.println(gps.location.lng());


          Serial.print(F("- altitude: "));
          if (gps.altitude.isValid()) {
            Firebase.RTDB.setFloat(&fbdo, "ESP/Latitude", gps.altitude.meters());
            Serial.println(gps.altitude.meters());
          } else {
            Serial.println(F("INVALID"));
          }
        } else {
          Serial.println(F("- location: INVALID"));
        }

        Serial.print(F("- speed: "));
        if (gps.speed.isValid()) {
          Serial.print(gps.speed.kmph());
          Serial.println(F(" km/h"));
        } else {
          Serial.println(F("INVALID"));
        }

        Serial.print(F("- GPS date&time: "));
        if (gps.date.isValid() && gps.time.isValid()) {
          Serial.print(gps.date.year());
          Serial.print(F("-"));
          Serial.print(gps.date.month());
          Serial.print(F("-"));
          Serial.print(gps.date.day());
          Serial.print(F(" "));
          Serial.print(gps.time.hour());
          Serial.print(F(":"));
          Serial.print(gps.time.minute());
          Serial.print(F(":"));
          Serial.println(gps.time.second());
        } else {
          Serial.println(F("INVALID"));
        }

      Serial.println();
    }
  }
  
  if (millis() > 10000 && gps.charsProcessed() < 10)
    Serial.println(F("No GPS data received: check wiring"));
  }

  
}