#include <Arduino.h>
#include <NewPing.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <Firebase_Arduino_WiFiNINA.h>

#include "wifi_data.h"
#include "firebase_data.h"

#define MOTOR_A 0
#define MOTOR_B 1

FirebaseData robotdo;

char ssid[] = SECRET_SSID; 
char pass[] = SECRET_PASS; 
char firebase_host[] = FIREBASE_HOST; 
char firebase_auth[] = FIREBASE_AUTH; 
int robot_status = 0;

String miscare_robot = "In curs de conectare...";

const int PWMA = 3; 
const int PWMB = 10; 
const int DIRA = 12; 
const int DIRB = 13; 
const int Trig = 8; 
const int Echo = 7; 
const int MAX_DISTANCE = 300;
const int AVOID_DISTANCE = 45;

int distance;

String path = "/robot";
NewPing sonar(Trig, Echo, MAX_DISTANCE);

void setup() {
 setupMotors();
 Serial.begin(9600);
 randomSeed(analogRead(0));
 Serial.flush();

 while (WiFi.status() != WL_CONNECTED) {
 connectToWiFi();

 delay(10000);
 }
 Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH, SECRET_SSID, SECRET_PASS);
}

void driveMotor(byte motor, byte dir, byte spd) {
 if (motor == MOTOR_A) {
 digitalWrite(DIRA, dir);
 analogWrite(PWMA, spd);
 } else if (motor == MOTOR_B) {
 digitalWrite(DIRB, dir);
 analogWrite(PWMB, spd);
 }
}

void setupMotors() {
 pinMode(PWMA, OUTPUT);
 pinMode(PWMB, OUTPUT);
 pinMode(DIRA, OUTPUT);
 pinMode(DIRB, OUTPUT);
}

void stopMotors() {

 digitalWrite(PWMA, LOW); 
 digitalWrite(PWMB, LOW); 
}

void connectToWiFi() {
 WiFi.begin(ssid, pass);
}

String displayWifi() {
 if (WiFi.status() == WL_CONNECTED) {
 return SECRET_SSID;
 } else return "In curs de conectare...";
}

String formatIP(IPAddress ip) {
 return String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
}

String displayIPWifi() {
 if (WiFi.status() == WL_CONNECTED) {
 IPAddress ip = WiFi.localIP();
 return formatIP(ip);
 } else return "In curs de conectare...";
}

String displayRSSIWifi() {
 if (WiFi.status() == WL_CONNECTED) {
 int rssi = WiFi.RSSI();
 if(rssi < -74)
 return String(rssi) + " dBm (Semnal slab)";
 else if((rssi < -64) && (rssi > -75))
 return String(rssi) + " dBm (Semnal bun)";
 else if((rssi > -64))
 return String(rssi) + " dBm (Semnal excelent)"; 
 } else return "In curs de conectare...";
}

String showStatus() {
 if (robot_status == 1) {
 return "Activ";
 } else return "Inactiv";
}

String showMovement() {
 return miscare_robot;

}

String showDistance() {
 return String(distance) + " cm";
}

void loop() {

 String statusMessage = showStatus();
 String movementMessage = showMovement();
 String distanceMessage = showDistance();
 String statusSSID = displayWifi();
 String statusRSSI = displayRSSIWifi();
 String statusIP = displayIPWifi(); 

 if (WiFi.status() == WL_CONNECTED) {
 
 distance = sonar.ping_cm();
 
 if (distance > AVOID_DISTANCE) {
 driveMotor(MOTOR_A, HIGH, 105);
 driveMotor(MOTOR_B, HIGH, 105);
 robot_status = 1;
 miscare_robot = "Inainte";

 Firebase.setString(robotdo, path + "/status_robot", statusMessage);
 Firebase.setString(robotdo, path + "/status_conexiune_wifi" + "/ssid", statusSSID);
 Firebase.setString(robotdo, path + "/status_conexiune_wifi" + "/ip", statusIP);
 Firebase.setString(robotdo, path + "/status_conexiune_wifi" + "/rssi", statusRSSI);
 Firebase.setString(robotdo, path + "/miscare", movementMessage); 
 Firebase.setString(robotdo, path + "/distanta", distanceMessage);
 delay(1000); 
 }
 if (distance <= AVOID_DISTANCE) { 
 
 int randNumber = random(1000);
 if (randNumber >= 500) {
 robot_status = 1;
 miscare_robot = "Viraj dreapta";
 
 Firebase.setString(robotdo, path + "/status_robot", statusMessage);
 Firebase.setString(robotdo, path + "/status_conexiune_wifi" + "/ssid", statusSSID);
 Firebase.setString(robotdo, path + "/status_conexiune_wifi" + "/ip", statusIP);
 Firebase.setString(robotdo, path + "/status_conexiune_wifi" + "/rssi", statusRSSI);
 Firebase.setString(robotdo, path + "/miscare", movementMessage); 
 Firebase.setString(robotdo, path + "/distanta", distanceMessage);

 while(distance <= AVOID_DISTANCE)
 {
 distance = sonar.ping_cm(); 
 driveMotor(MOTOR_A, HIGH, 100);
 driveMotor(MOTOR_B, LOW, 100);
 delay(500);
 }
 }
 else {
 robot_status = 1;
 miscare_robot = "Viraj stanga";
 
 Firebase.setString(robotdo, path + "/status_robot", statusMessage);
 Firebase.setString(robotdo, path + "/status_conexiune_wifi" + "/ssid", statusSSID);
 Firebase.setString(robotdo, path + "/status_conexiune_wifi" + "/ip", statusIP);
 Firebase.setString(robotdo, path + "/status_conexiune_wifi" + "/rssi", statusRSSI);
 Firebase.setString(robotdo, path + "/miscare", movementMessage); 
 Firebase.setString(robotdo, path + "/distanta", distanceMessage);
 
 while(distance <= AVOID_DISTANCE)
 {
 distance = sonar.ping_cm();
 driveMotor(MOTOR_A, LOW, 100);
 driveMotor(MOTOR_B, HIGH, 100); 
 delay(500);
 }
 }
 }
 Serial.println("Error: "+robotdo.errorReason());
} else {
 stopMotors();
}
}