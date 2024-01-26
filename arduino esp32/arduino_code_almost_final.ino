#include <Arduino.h>
#line 1 "C:\\Users\\stlp\\AppData\\Local\\Temp\\.arduinoIDE-unsaved2023116-4188-cww4wo.mhoe\\sketch_dec6a\\sketch_dec6a.ino"
#include <Wire.h>
#include <ArduinoBLE.h>
#include "DHT.h"
#include <string>
#include <vector>
#define CHARACTERISTIC_SIZE  19// Change based on your requirement
BLEService customService("20336bc0-c8f9-4de7-b637-a68b7ef33fc9");  // 1816 is the defined UUID for cycling tech...
BLECharacteristic dataCharacteristic("23336bc0-c8f9-4de7-b637-a68b7ef33fc9",  // Custom characteristic UUID
                                   BLERead | BLENotify | BLEWrite, 
                                   CHARACTERISTIC_SIZE);  // Characteristic value length


BLEDescriptor dataDescriptor("00002902-0000-1000-8000-00805f9b34fb", "All data");  // Used for enabling notifications.


uint8_t packet[CHARACTERISTIC_SIZE];
const int soilMoistureReadPin = 32;
const int soilMoisturePowerPin = 33;
const int lightSensorPin = 34;
const int dhtPin = 27;  // Digital pin for DHT22
const int waterLevel = 4; // Read = Yellow, Black
const int solenoid = 25; // Power = Green, Ground = Yellow

DHT dht(dhtPin, DHT22);

void setup() {
  Serial.begin(115200);
  dht.begin(dhtPin);
  pinMode(soilMoistureReadPin, INPUT);
  pinMode(lightSensorPin, INPUT);
  pinMode(soilMoisturePowerPin, OUTPUT);
  pinMode(dhtPin, INPUT);
  pinMode(waterLevel, INPUT);
  pinMode(solenoid, OUTPUT);
  delay(1000);
  Serial.println("Starting...");

  // Initialize BLE hardware
  if (!BLE.begin()) {
    while (1) {
      Serial.println("Starting BLE failed!");
      delay(1000);
    }
  }
  
  // Set the local name and service information
  BLE.setLocalName("SPLiSHY");
  BLE.setAdvertisedService(customService);
  // Add custom characteristic
  customService.addCharacteristic(dataCharacteristic);

  dataCharacteristic.addDescriptor(dataDescriptor);

  BLE.addService(customService);
  
  // Start advertising
  BLE.advertise();
  Serial.println("Bluetooth device active, waiting for connections...");
}



void loop() {
  BLEDevice central = BLE.central();
  Serial.println("Waiting to connect to central.");
  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());
    String concat = "";
    while (central.connected()) {
      if (dataCharacteristic.written()) {
        // Read the received data
        Serial.println("made it here");
        const uint8_t* receivedData = dataCharacteristic.value();
        String data = "";
        Serial.print("Received data: ");
        for(int i = 0; i < dataCharacteristic.valueLength(); i++){
          // Serial.print(receivedData[i] - 48);
          // Serial.print(" ");
          data += receivedData[i] -48;
          Serial.println(data);
        }
        int wateringTime = data.toInt();
        Serial.print("this is an int: ");
        Serial.println(wateringTime);
        digitalWrite(solenoid, HIGH);
        delay(wateringTime);
        digitalWrite(solenoid,LOW);

      }
      delay(100);
      digitalWrite(soilMoisturePowerPin, HIGH);
      int soilMoistureValue = analogRead(soilMoistureReadPin);
      concat += String (soilMoistureValue);
      concat += ",";
      int lightSensorValue = analogRead(lightSensorPin);
      if(lightSensorValue < 1000 ){
        lightSensorValue = 1;
      }
      else if(lightSensorValue < 2000){
        lightSensorValue = 2;
      }
      else if(lightSensorValue < 3000){
        lightSensorValue = 3;
      }
      else{
        lightSensorValue = 4;
      }
      concat += String (lightSensorValue);
      int waterLow = analogRead(waterLevel);
      if(waterLow < 1000) {
        waterLow = 0;
      }
      else{
        waterLow = 1;
      }
      concat += String (waterLow);
      concat += ",";
      float temperature = dht.readTemperature();
      concat += String (temperature);
      concat = concat.substring(0,concat.length() -1);
      concat += ",";
      float humidity = dht.readHumidity();
      concat += String (humidity);
      concat = concat.substring(0,concat.length() -1);
      concat += ",";
      Serial.println(soilMoistureValue);
      // String temp = "Can you read this";
      concat.getBytes(packet, CHARACTERISTIC_SIZE);
      dataCharacteristic.writeValue(packet, CHARACTERISTIC_SIZE);
      digitalWrite(soilMoisturePowerPin, LOW);
      delay(400);  // check sensor data every 100ms
      Serial.println(waterLow);
      Serial.print("temp: ");
      Serial.println(temperature);
      Serial.print("humidity: ");
      Serial.println(humidity);
      Serial.println(concat);
      concat = "";
    }
      
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
    delay(1000);
  }
}

