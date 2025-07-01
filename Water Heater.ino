#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Pin definitions
#define TRIG_NORMAL   2
#define ECHO_NORMAL   4
#define TRIG_HOT      16
#define ECHO_HOT      17
#define RELAY_PUMP    26
#define RELAY_HEATER  25
#define ONE_WIRE_BUS  27
#define BUZZER        19

// Ultrasonic sensor thresholds
#define EMPTY_DISTANCE  30  // cm, distance indicating empty tank
#define FULL_DISTANCE   10   // cm, distance indicating full tank

// Temperature threshold
#define HOT_TEMPERATURE 60 // degrees Celsius

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Temperature sensor setup
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  // Initialize serial communication
  Serial.begin(115200);

  // Initialize LCD
  lcd.init();
  lcd.backlight();

  // Initialize ultrasonic sensor pins
  pinMode(TRIG_NORMAL,  OUTPUT);
  pinMode(ECHO_NORMAL,  INPUT);
  pinMode(TRIG_HOT,     OUTPUT);
  pinMode(ECHO_HOT,     INPUT);
  pinMode(BUZZER,       OUTPUT);

  // Initialize relays
  pinMode(RELAY_PUMP,         OUTPUT);
  pinMode(RELAY_HEATER,       OUTPUT);
  digitalWrite(RELAY_PUMP,    LOW);
  digitalWrite(RELAY_HEATER,  LOW);
  delay(2000);
  digitalWrite(RELAY_PUMP,    HIGH);
  digitalWrite(RELAY_HEATER,  HIGH);
  delay(1000);

  // Initialize temperature sensor
  sensors.begin();
}

void loop() {
  // Read ultrasonic sensors
  int distanceNormal = readUltrasonic(TRIG_NORMAL, ECHO_NORMAL);
  int distanceHot = readUltrasonic(TRIG_HOT, ECHO_HOT);

  // Display status on LCD
  lcd.setCursor(0, 0);
  if (distanceNormal > EMPTY_DISTANCE) {
    lcd.print("Normal: Empty ");
  } else {
    lcd.print("Normal: OK    ");
  }

  lcd.setCursor(0, 1);
  if (distanceHot > EMPTY_DISTANCE) {
    lcd.print("Hot: Empty    ");
  } else {
    lcd.print("Hot: OK       ");
  }

  // Control pump
  if (distanceHot > EMPTY_DISTANCE && distanceNormal <= EMPTY_DISTANCE) {
    digitalWrite(RELAY_PUMP, LOW); // Turn on pump
    digitalWrite(BUZZER, LOW);
  } else if (distanceHot <= FULL_DISTANCE) {
    digitalWrite(RELAY_PUMP, HIGH);  // Turn off pump
    digitalWrite(BUZZER, HIGH);
  }

  // Read temperature sensor
  sensors.requestTemperatures();
  float temperature = sensors.getTempCByIndex(0);

  // Control heater
  if (distanceHot <= FULL_DISTANCE) {
    if (temperature < HOT_TEMPERATURE) {
      digitalWrite(RELAY_HEATER, LOW); // Turn on heater
    } else {
      digitalWrite(RELAY_HEATER, HIGH);  // Turn off heater
    }
  }

  // Debug output
  Serial.print("Distance Normal: ");
  Serial.print(distanceNormal);
  Serial.print(" cm, Distance Hot: ");
  Serial.print(distanceHot);
  Serial.print(" cm, Temperature: ");
  Serial.print(temperature);
  Serial.println(" C");

  delay(1000);
}

// Function to read distance from ultrasonic sensor
int readUltrasonic(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  int distance = duration * 0.034 / 2; // Convert to cm
  return distance;
}