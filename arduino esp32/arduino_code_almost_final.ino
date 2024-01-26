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
// dataCharacteristic.setWriteType(BLEWrite);
// BLECharacteristic tempCharacteristic("845efa95-b66f-4a4e-94d2-d115a0e62074",  // Custom characteristic UUID
//                                    BLERead | BLENotify, 
//                                    CHARACTERISTIC_SIZE);  // Characteristic value length

// BLECharacteristic humidityCharacteristic("87214acd-c623-4ca1-be44-958b10d7f4a6",  // Custom characteristic UUID
//                                    BLERead | BLENotify, 
//                                    CHARACTERISTIC_SIZE);  // Characteristic value length

// BLECharacteristic lightCharacteristic("242811be-4365-49ea-9526-9e57bb1ba371",  // Custom characteristic UUID
//                                    BLERead | BLENotify, 
//                                    CHARACTERISTIC_SIZE);  // Characteristic value length

// BLECharacteristic solenoidCharacteristic("6217b278-33c5-4c47-baf6-bf92952b0cb8",  // Custom characteristic UUID
//                                    BLERead | BLENotify, 
//                                    CHARACTERISTIC_SIZE);  // Characteristic value length

// BLECharacteristic waterCharacteristic("be52316f-4b76-49c4-b033-6d3108a9d129",  // Custom characteristic UUID
//                                    BLERead | BLENotify, 
//                                    CHARACTERISTIC_SIZE);  // Characteristic value length

BLEDescriptor dataDescriptor("00002902-0000-1000-8000-00805f9b34fb", "All data");  // Used for enabling notifications.
// BLEDescriptor waterDescriptor("e6fa694e-c959-4b0f-9460-2773283c08f2", "Water Level");  // Used for enabling notifications.
// BLEDescriptor tempDescriptor("f7dd640b-1f4e-4c59-8ad9-ebc3e6ef9cbf", "Temperature");  // Used for enabling notifications.
// BLEDescriptor humidityDescriptor("194fc316-6d1d-4747-8b13-088c224164e5", "Humidity");  // Used for enabling notifications.
// BLEDescriptor solenoidDescriptor("a685a1da-7f3b-4900-ae93-4b67cd8d640b", "Solenoid");  // Used for enabling notifications.
// BLEDescriptor lightDescriptor("be62f4be-529d-4e88-8d5e-845c403a1ab1", "Light Level");  // Used for enabling notifications.

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
  // customService.addCharacteristic(waterCharacteristic);
  // customService.addCharacteristic(tempCharacteristic);
  // customService.addCharacteristic(humidityCharacteristic);
  // customService.addCharacteristic(solenoidCharacteristic);
  // customService.addCharacteristic(lightCharacteristic);
  dataCharacteristic.addDescriptor(dataDescriptor);
  // waterCharacteristic.addDescriptor(waterDescriptor);
  // tempCharacteristic.addDescriptor(tempDescriptor);
  // humidityCharacteristic.addDescriptor(humidityDescriptor);
  // solenoidCharacteristic.addDescriptor(solenoidDescriptor);
  // lightCharacteristic.addDescriptor(lightDescriptor);
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
      // initialize packet data array
      // const uint8_t* currentData = dataCharacteristic.value();
      // uint8_t data = *currentData;
      // for(int i = 0; i < dataCharacteristic.valueLength(); i++){
      //   data = currentData[i];
      //   Serial.print(data);
      //   Serial.print(", ");
      // }
      // Serial.print("Data: ");
      // Serial.println(data);
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
        // Serial.println();
        // Serial.println(receivedData.c_str());
        // Process the received data as needed
        // Add your code here to handle the received data
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

