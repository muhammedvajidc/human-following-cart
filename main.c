
#include <NewPing.h>           
#include <Servo.h>             
#include <AFMotor.h>           
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Motor and Sensor Definitions
#define RIGHT A2 // Right IR sensor connected to analog pin A2 of Arduino Uno
#define LEFT A3 // Left IR sensor connected to analog pin A3 of Arduino Uno
#define TRIGGER_PIN A1 // Trigger pin connected to analog pin A1 of Arduino Uno
#define ECHO_PIN A0 // Echo pin connected to analog pin A0 of Arduino Uno
#define MAX_DISTANCE 200 // Maximum ping distance

// RFID Definitions
#define RST_PIN 9
#define SS_PIN 10

// Global Variables
unsigned int distance = 0; // Variable to store ultrasonic sensor distance
unsigned int Right_Value = 0; // Variable to store Right IR sensor value
unsigned int Left_Value = 0; // Variable to store Left IR sensor value
float total = 0.0; // Shopping cart total

// Object Initializations
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // Ultrasonic sensor
MFRC522 mfrc522(SS_PIN, RST_PIN); // RFID reader
LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD display
Servo myservo; // Servo motor

// Create motor objects
AF_DCMotor Motor1(1, MOTOR12_1KHZ);
AF_DCMotor Motor2(2, MOTOR12_1KHZ);
AF_DCMotor Motor3(3, MOTOR34_1KHZ);
AF_DCMotor Motor4(4, MOTOR34_1KHZ);

// Product database
struct Item {
  String uid;
  String name;
  float price;
};

Item items[] = {
  {"4A 6C 79 12", "biscuit", 30},
  {"E5 F6 G7 H8", "milk", 1.75},
  {"I9 J0 K1 L2", "pen", 3.20},
};

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  
  // Initialize servo
  myservo.attach(10);
  initializeServo();
  
  // Initialize sensors
  pinMode(RIGHT, INPUT);
  pinMode(LEFT, INPUT);
  
  // Initialize RFID
  while (!Serial); // For Leonardo/Micro
  SPI.begin();
  mfrc522.PCD_Init();
  
  // Initialize LCD
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  delay(1000);
  displayWelcomeMessage();
}

void loop() {
  // Check for RFID items
  checkRFID();
  
  // Perform obstacle avoidance
  obstacleAvoidance();
}

void initializeServo() {
  for(int pos = 90; pos <= 180; pos += 1) { // goes from 90 to 180 degrees
    myservo.write(pos); // tell servo to go to position
    delay(15); // wait 15ms
  } 
  for(int pos = 180; pos >= 0; pos -= 1) { // goes from 180 to 0 degrees
    myservo.write(pos); // tell servo to go to position
    delay(15); // wait 15ms
  }
  for(int pos = 0; pos <= 90; pos += 1) { // goes from 0 to 90 degrees
    myservo.write(pos); // tell servo to go to position
    delay(15); // wait 15ms
  }
}

void checkRFID() {
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  
  String uid = getUID();
  Serial.print("Card UID: ");
  Serial.println(uid);
  
  bool found = false;
  for (int i = 0; i < sizeof(items)/sizeof(items[0]); i++) {
    if (uid == items[i].uid) {
      found = true;
      total += items[i].price;
      displayItem(items[i].name, items[i].price, total);
      break;
    }
  }
  
  if (!found) {
    lcd.clear();
    lcd.print("Item not found");
    lcd.setCursor(0, 1);
    lcd.print("Total: $");
    lcd.print(total);
    delay(2000);
    displayWelcomeMessage();
  }
  
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

String getUID() {
  String uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uid += String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    uid += String(mfrc522.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  uid.trim();
  return uid;
}

void displayWelcomeMessage() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Smart Cart Ready");
  lcd.setCursor(0, 1);
  lcd.print("Total: $");
  lcd.print(total);
}

void displayItem(String name, float price, float total) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(name);
  lcd.print(" $");
  lcd.print(price);
  
  lcd.setCursor(0, 1);
  lcd.print("Total: $");
  lcd.print(total);
  delay(2000);
  displayWelcomeMessage();
}

void obstacleAvoidance() {
  delay(50);
  distance = sonar.ping_cm();
  Right_Value = digitalRead(RIGHT);
  Left_Value = digitalRead(LEFT);
  
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print("cm | Right: ");
  Serial.print(Right_Value);
  Serial.print(" | Left: ");
  Serial.println(Left_Value);
  
  if((distance > 1) && (distance < 15)) {
    // Move Forward
    moveMotors(FORWARD, FORWARD, FORWARD, FORWARD, 130);
  } 
  else if((Right_Value == 0) && (Left_Value == 1)) {
    // Turn Left
    moveMotors(FORWARD, FORWARD, BACKWARD, BACKWARD, 150);
    delay(150);
  } 
  else if((Right_Value == 1) && (Left_Value == 0)) {
    // Turn Right
    moveMotors(BACKWARD, BACKWARD, FORWARD, FORWARD, 150);
    delay(150);
  } 
  else if(distance > 15) {
    // Stop
    stopMotors();
  }
}

void moveMotors(uint8_t m1, uint8_t m2, uint8_t m3, uint8_t m4, int speed) {
  Motor1.setSpeed(speed);
  Motor1.run(m1);
  Motor2.setSpeed(speed);
  Motor2.run(m2);
  Motor3.setSpeed(speed);
  Motor3.run(m3);
  Motor4.setSpeed(speed);
  Motor4.run(m4);
}

void stopMotors() {
  Motor1.run(RELEASE);
  Motor2.run(RELEASE);
  Motor3.run(RELEASE);
  Motor4.run(RELEASE);
}
