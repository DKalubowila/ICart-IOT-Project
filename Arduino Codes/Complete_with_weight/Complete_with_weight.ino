#include <WiFi.h>
#include <FirebaseESP32.h>
#include <SPI.h>
#include <MFRC522.h>
#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

#define WIFI_SSID "Test"
#define WIFI_PASSWORD "12345678"

#define API_KEY "M2GWhWMhbg2VPL0PHUDZZnkxNwfLc5zC0XBKz1Ew"
#define DATABASE_URL "i-cart-ce9ba-default-rtdb.firebaseio.com"

#define SS_PIN  21  /* Slave Select Pin */
#define RST_PIN 22  /* Reset Pin for RC522 */

#define FRONT_IR_PIN 4   // Pin connected to front IR sensor
#define BACK_IR_PIN 5    // Pin connected to back IR sensor

#define Relay   2   /* Pin D2 for LED */

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

MFRC522 mfrc522(SS_PIN, RST_PIN);

int count = 0;
int lastCount = 0;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 3000;

//pins:
const int HX711_dout = 16; //mcu > HX711 dout pin
const int HX711_sck = 15; //mcu > HX711 sck pin

//HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);

const int calVal_eepromAdress = 0;
unsigned long t = 0;


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
  pinMode(Relay, OUTPUT);  /* LED Pin set as output */
  SPI.begin();            /* SPI communication initialized */
  mfrc522.PCD_Init();     /* RFID sensor initialized */

  Serial.println("Put your card to the reader...");
  Serial.println();

LoadCell.begin();
  //LoadCell.setReverseOutput(); //uncomment to turn a negative output value to positive
  float calibrationValue; // calibration value (see example file "Calibration.ino")
  calibrationValue = 817.45; // uncomment this if you want to set the calibration value in the sketch
#if defined(ESP8266)|| defined(ESP32)
  //EEPROM.begin(512); // uncomment this if you use ESP8266/ESP32 and want to fetch the calibration value from eeprom
#endif
  //EEPROM.get(calVal_eepromAdress, calibrationValue); // uncomment this if you want to fetch the calibration value from eeprom

  unsigned long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag()) {
    while (1);
  }
  else {
    LoadCell.setCalFactor(calibrationValue); // set calibration value (float)
  } 
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
    // Avoid sending negative values to the database
    int nonNegativeCount = count >= 0 ? count : 0;
    
    // Firebase data sending
    Firebase.setInt(fbdo, "/ItemCount", nonNegativeCount);
    delay(200); // Ensure some delay before accessing data
    Serial.printf("Count: %d\n", nonNegativeCount);
    
    lastCount = count;
}

  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    Serial.print("UID tag :");
    String content = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(mfrc522.uid.uidByte[i], HEX);
      content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    Serial.println();
    Serial.print("Message : ");
    content.toUpperCase();
    if (content.substring(1) == "12 75 E2 34") {
      Firebase.setInt(fbdo, "/Cart01", 1);
      Firebase.setInt(fbdo, "/CartStatus", 1);
      digitalWrite (Relay, HIGH);
    } else if (content.substring(1) == "73 D2 68 0F") {
      Firebase.setInt(fbdo, "/Cart02", 1);
      Firebase.setInt(fbdo, "/CartStatus", 1);
      digitalWrite (Relay, HIGH);
    } else {
      Firebase.setInt(fbdo, "/Cart01", 0);
      Firebase.setInt(fbdo, "/Cart02", 0);
    }
    Serial.println("RFID data sent to Firebase");
    Serial.println();
    
    delay(500);
  }
  
  // Check for user input via Firebase
  if (Firebase.getInt(fbdo, "/CartStatus")) {
    int controlValue = fbdo.to<int>();
    if (controlValue == 1) {
      // Turn on the LED
      digitalWrite(Relay, HIGH);
    } else if (controlValue == 0) {
      // Turn off the LED
      digitalWrite(Relay, LOW);
    }
  }

  // Load cell data sending to Firebase
if (LoadCell.update()) {
  float weight = LoadCell.getData();
  
  // Only send weight data if it's positive
  if (weight >= 0) {
    // Format weight with one decimal place
    char formattedWeight[10]; // Adjust size as needed
    snprintf(formattedWeight, sizeof(formattedWeight), "%.0f", weight);
    
    // Send formatted weight data to Firebase
    Firebase.setString(fbdo, "/TotalWeight", formattedWeight);

    // Print weight to serial monitor (optional, adjust interval)
    if (millis() - t >= 1000) {  // Print every second
      Serial.print("Load cell output value: ");
      Serial.println(formattedWeight);
      t = millis();
    }
  }
}
}
