#include <ArduinoBLE.h>
#include "LSM6DS3.h"
//#include "Wire.h"

// THIS IS THE PERIPHERAL (sleeping then sending)
// IMU code from: Examples > Seeed Arduino LSM6DS3 > HighLevelExample.ino
// IMU reference: https://wiki.seeedstudio.com/XIAO-BLE-Sense-IMU-Usage/
// Bluetooth code from: https://wiki.seeedstudio.com/XIAO-BLE-Sense-Bluetooth-Usage/


// Initialize definitions for inertial measurement
LSM6DS3 myIMU(I2C_MODE, 0x6A);    //I2C device address 0x6A
float xAccel, yAccel, zAccel;

// Initialize bluetooth service
BLEService legoGService("19B10000-E8F2-537E-4F6C-D104768A1214"); // Bluetooth® Low Energy LED Service

// Bluetooth® Low Energy data Characteristic - custom 128-bit UUID, read and writable by central
BLEFloatCharacteristic xAccelerationCharacteristic("0695f8fb-8576-4f6e-b6cf-ce36a0a60bc2", BLERead);
BLEFloatCharacteristic yAccelerationCharacteristic("1c1276b5-bf59-42ea-ba31-efd4027e028c", BLERead);
BLEFloatCharacteristic zAccelerationCharacteristic("6102620a-34c6-4cc5-9f78-a04a73169b1f", BLERead | BLENotify);


void setup() {
  Serial.begin(9600);
  while (!Serial);

  //Call .begin() to configure the IMUs
  if (myIMU.begin() != 0) {
      Serial.println("Device error");
  } else {
      Serial.println("Device OK!");
  }


  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  // set advertised local name and service UUID:
  BLE.setLocalName("lego-G");
  BLE.setAdvertisedService(legoGService);

  // add the characteristic to the service
  legoGService.addCharacteristic(xAccelerationCharacteristic);
  legoGService.addCharacteristic(yAccelerationCharacteristic);
  legoGService.addCharacteristic(zAccelerationCharacteristic);

  // add service
  BLE.addService(legoGService);

  // set the initial value for the characeristic:
  xAccelerationCharacteristic.writeValue(0);
  yAccelerationCharacteristic.writeValue(0);
  zAccelerationCharacteristic.writeValue(0);

  // start advertising
  BLE.advertise();

  // print address
  Serial.print("Address: ");
  Serial.println(BLE.address());

  Serial.println("lego-G Peripheral");
}

void loop() {



  // listen for Bluetooth® Low Energy peripherals to connect:
  BLEDevice central = BLE.central();

  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());
    // turn on the LED to indicate the connection:
    digitalWrite(LED_BUILTIN, HIGH);

    // while the central is still connected to peripheral:
    while (central.connected()) {
      // read sensor values
      xAccel = myIMU.readFloatAccelX();
      yAccel = myIMU.readFloatAccelY();
      zAccel = myIMU.readFloatAccelZ();
      
      Serial.print("\nAccelerometer:\n");
      Serial.print(" X1 = ");
      Serial.println(xAccel, 4);
      Serial.print(" Y1 = ");
      Serial.println(yAccel, 4);
      Serial.print(" Z1 = ");
      Serial.println(zAccel, 4);

      // set the updated values for the characeristic:
      xAccelerationCharacteristic.writeValue(xAccel);
      yAccelerationCharacteristic.writeValue(yAccel);
      zAccelerationCharacteristic.writeValue(zAccel);
      
      delay(500);
    }

    // when the central disconnects, print it out:
    digitalWrite(LED_BUILTIN, LOW);
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }
}


void readPrintSensor(){
    // GET SENSOR DATA
  //Accelerometer
  Serial.print("\nAccelerometer:\n");
  Serial.print(" X1 = ");
  Serial.println(myIMU.readFloatAccelX(), 4);
  Serial.print(" Y1 = ");
  Serial.println(myIMU.readFloatAccelY(), 4);
  Serial.print(" Z1 = ");
  Serial.println(myIMU.readFloatAccelZ(), 4);

  //Gyroscope
  Serial.print("\nGyroscope:\n");
  Serial.print(" X1 = ");
  Serial.println(myIMU.readFloatGyroX(), 4);
  Serial.print(" Y1 = ");
  Serial.println(myIMU.readFloatGyroY(), 4);
  Serial.print(" Z1 = ");
  Serial.println(myIMU.readFloatGyroZ(), 4);

  //Thermometer
  Serial.print("\nThermometer:\n");
  Serial.print(" Degrees C1 = ");
  Serial.println(myIMU.readTempC(), 4);
  Serial.print(" Degrees F1 = ");
  Serial.println(myIMU.readTempF(), 4);
}