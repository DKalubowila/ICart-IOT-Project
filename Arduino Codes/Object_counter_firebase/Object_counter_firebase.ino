#include <WiFi.h>
#include <FirebaseESP32.h>

#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

#define WIFI_SSID "Test"
#define WIFI_PASSWORD "12345678"

#define API_KEY "M2GWhWMhbg2VPL0PHUDZZnkxNwfLc5zC0XBKz1Ew"
#define DATABASE_URL "i-cart-ce9ba-default-rtdb.firebaseio.com"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

#define FRONT_IR_PIN 4   // Pin connected to front IR sensor
#define BACK_IR_PIN 5    // Pin connected to back IR sensor

int count = 0;
int lastCount = 0;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 3000;

void setup() {
  Serial.begin(115200);
  delay(2000);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
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

  Firebase.begin(DATABASE_URL, API_KEY);
  Firebase.setDoubleDigits(5);

  pinMode(FRONT_IR_PIN, INPUT);
  pinMode(BACK_IR_PIN, INPUT);
}

void loop() {
  int frontIRState = digitalRead(FRONT_IR_PIN);
  int backIRState = digitalRead(BACK_IR_PIN);

  if (frontIRState == HIGH && backIRState == LOW && millis() - lastDebounceTime > debounceDelay) {
    count++;
    lastDebounceTime = millis();
  } else if (backIRState == HIGH && frontIRState == LOW && millis() - lastDebounceTime > debounceDelay) {
    count--;
    lastDebounceTime = millis();
  }

  if (count != lastCount) {
    // Firebase data sending
    Firebase.setInt(fbdo, "/test/count", count);
    delay(200); // Ensure some delay before accessing data
    Serial.printf("Count: %d\n", count);
    lastCount = count;
  }
}
