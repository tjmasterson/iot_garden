#pragma once

#define DEBUG false

#define PH_RX 3
#define PH_TX 2
#define EC_RX 9
#define EC_TX 8
#define TEMP_PIN 11

#define ARDUINO_ADDRESS 0x08;
#define DEVICE_ID "Water_Monitor";
#define VERSION  "0.1.0"

#define EC_SENSOR_ID "ec_sensor"
#define PH_SENSOR_ID "ph_sensor"
#define TEMP_SENSOR_ID "temp_sensor"
#define TDS_SENSOR_ID "tds_sensor"
#define SAL_SENSOR_ID "sal_sensor"

//#define TEMP_SENSOR_NAME "Temperature"
//#define EC_SENSOR_NAME "Electrical_Conductivity"
//#define PH_SENSOR_NAME "pH"
//#define TDS_SENSOR_NAME "Total_Dissolved_Solids"
//#define SAL_SENSOR_NAME "Salinity"

#define START_CONVERT 0
#define READ_TEMP 1
