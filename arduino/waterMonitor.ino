/*********************************************************************
 * waterMonitor.ino
 *
 * Description:
 * Used to monitor water quality including ph, temperature, and ec .
 *
 * Hardware platform   : Arduino M0 Or Arduino Mega2560
 * Sensor pin:
 * EC:          D2,D3
 * PH:          D8,D9
 * Temperature: D11
 *
 * author:  tjmasterson
 * version: 0.1.0
 **********************************************************************/
#include "config.h"
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <OneWire.h>
#include <AltSoftSerial.h>

SoftwareSerial phSerial(PH_RX, PH_TX);
AltSoftSerial ecSerial(EC_RX, EC_TX);

float temp;
float ec;
float ph;
float tds;
float sal;

String output = "";
String pHSensorString = "";
String ecSensorString = "";
String setTemp = "T,";
boolean pHSensorStringComplete = false;
boolean ecSensorStringComplete = false;
unsigned long tempSampleInterval = 850;
unsigned long tempSampleTime;

OneWire* oneWire;


void setup() {
  Serial.begin(115200);
  phSerial.begin(9600);
  ecSerial.begin(9600);

  oneWire = new OneWire(TEMP_PIN);

  pHSensorString.reserve(30);
  ecSensorString.reserve(30);
  output.reserve(150);
}

void loop() {
  if ( millis () - tempSampleTime >= tempSampleInterval) {
    tempSampleTime = millis ();
    temp = tempProcess(READ_TEMP);  // read the current temperature from the  DS18B20
    tempProcess(START_CONVERT);
    if (temp > 0) {
      String tempCommand = setTemp + String(temp);
      phSerial.print(tempCommand);
      phSerial.print('\r');
      ecSerial.print(tempCommand);
      ecSerial.print('\r');
      tempCommand = "";
    }
  }

  phSerial.listen();
  if (phSerial.available() > 0) {

    char inchar = (char)phSerial.read();
    pHSensorString += inchar;
    if (inchar == '\r') {
      pHSensorStringComplete = true;
    }
  }

  ecSerial.listen();
  if (ecSerial.available() > 0) {
    char inchar = (char)ecSerial.read();
    ecSensorString += inchar;
    if (inchar == '\r') {
      ecSensorStringComplete = true;
    }
  }

  if (ecSensorStringComplete == true) {
    if (isdigit(ecSensorString[0]) == true) {
      char sensorstring_array[30];
      ecSensorString.toCharArray(sensorstring_array, 30);
      char * _ec;
      char * _tds;
      char * _sal;
      _ec = strtok(sensorstring_array, ",");
      _tds = strtok(NULL, ",");
      _sal = strtok(NULL, ",");
      ec = atof(_ec);
      tds = atof(_tds);
      sal = atof(_sal);
    }
    ecSensorStringComplete = false;
    ecSensorString = "";
  }

  if (pHSensorStringComplete == true) {
    if (isdigit(pHSensorString[0]) == true) {
      ph = pHSensorString.toFloat();
    }
    pHSensorString = "";
    pHSensorStringComplete = false;
  }

  while (Serial.available() > 0) {
    int incomingValue = Serial.read(); //Read the incoming data and store it into variable incomingValue
    if (incomingValue) {       // if data is byte get write data to serial line
      sendData();
      Serial.print(output);
      Serial.flush();
      output = "";
    }
  }
}

double tempProcess(bool ch)
{
  static byte data[12];
  static byte addr[8];
  static float TemperatureSum;
  if (!ch) {
    if (!oneWire->search(addr)) {
      if (DEBUG) {
        Serial.println("no temperature sensors on chain, reset search!");
      }
      oneWire->reset_search();
      return 0;
    }
    if (OneWire::crc8(addr, 7) != addr[7]) {
      if (DEBUG) {
        Serial.println("CRC is not valid!");
      }
      return 0;
    }
    if (addr[0] != 0x10 && addr[0] != 0x28) {
      if (DEBUG) {
        Serial.println("Device is not recognized!");
      }
      return 0;
    }
    oneWire->reset();
    oneWire->select(addr);
    oneWire->write(0x44, 1); // start conversion, with parasite power on at the end
  }
  else {
    byte present = oneWire->reset();
    oneWire->select(addr);
    oneWire->write(0xBE); // Read Scratchpad
    for (int i = 0; i < 9; i++) { // we need 9 bytes
      data[i] = oneWire->read();
    }
    oneWire->reset_search();
    byte MSB = data[1];
    byte LSB = data[0];
    float tempRead = ((MSB << 8) | LSB); //using two's compliment
    TemperatureSum = tempRead / 16;
  }
  return TemperatureSum;
}

//  output looks like
// '{"device":"Water_Monitor","version":"0.1.0","temp_sensor":21.875,"ph_sensor":7.163,"ec_sensor":390.4,"tds_sensor":211,"sal_sensor":0.19}'
void sendData() {

  StaticJsonDocument<256> doc;

  doc["device"] = DEVICE_ID;
  doc["version"] = VERSION;

  doc[TEMP_SENSOR_ID] = temp;
  doc[PH_SENSOR_ID] = ph;
  doc[EC_SENSOR_ID] = ec;
  doc[TDS_SENSOR_ID] = tds;
  doc[SAL_SENSOR_ID] = sal;

  serializeJson(doc, output);
}
