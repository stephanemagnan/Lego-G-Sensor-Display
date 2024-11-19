#include <ArduinoBLE.h>
//#include <U8x8lib.h>
//#include <Wire.h>

// THIS IS THE CENTRAL (always scanning)

// initialize readings
float xAccel = 0;
float yAccel = 0;
float zAccel = 0;
float resAccel = 0; 

void setup() {
  Serial.begin(9600);
  while (!Serial);


  // initialize the Bluetooth® Low Energy hardware
  BLE.begin();

  Serial.println("Bluetooth® Low Energy Central - Lego G Central");


  // start scanning for peripherals
  //BLE.scanForUuid("19b10000-e8f2-537e-4f6c-d104768a1214");
  // start scanning for peripherals
  BLE.scanForName("lego-G");
}

void loop() {


  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // discovered a peripheral, print out address, local name, and advertised service
    Serial.print("Found ");
    Serial.print(peripheral.address());
    Serial.print(" '");
    Serial.print(peripheral.localName());
    Serial.print("' ");
    Serial.print(peripheral.advertisedServiceUuid());
    Serial.println();

    if (peripheral.localName() == "lego-G") {
      // stop scanning
      BLE.stopScan();

      readLegoGSensor(peripheral);

      // peripheral disconnected, start scanning again
      //BLE.scanForName("lego-G");
    }
  }
  delay(1000);
}

void readLegoGSensor(BLEDevice peripheral){

  // connect to the peripheral
  Serial.println("Connecting ...");
  if (peripheral.connect()) {
    Serial.println("Connected");
  } else {
    Serial.println("Failed to connect!");
    //return;
  }

  // discover peripheral attributes
  Serial.println("Discovering attributes ...");
  if (peripheral.discoverAttributes()) {
    Serial.println("Attributes discovered");
  } else {
    Serial.println("Attribute discovery failed!");
    peripheral.disconnect();
    return;
  }

  // retrieve the accelerometer characteristic
  BLECharacteristic xAccelCharacteristic = peripheral.characteristic("0695f8fb-8576-4f6e-b6cf-ce36a0a60bc2");
  BLECharacteristic yAccelCharacteristic = peripheral.characteristic("1c1276b5-bf59-42ea-ba31-efd4027e028c");
  BLECharacteristic zAccelCharacteristic = peripheral.characteristic("6102620a-34c6-4cc5-9f78-a04a73169b1f");


  // subscribe to the simple key characteristic
  Serial.println("Subscribing to zAccel characteristic ...");
  if (!zAccelCharacteristic) {
    Serial.println("no zAccel characteristic found!");
    peripheral.disconnect();
    return;
  } else if (!zAccelCharacteristic.canSubscribe()) {
    Serial.println("zAccel key characteristic is not subscribable!");
    peripheral.disconnect();
    return;
  } else if (!zAccelCharacteristic.subscribe()) {
    Serial.println("subscription failed!");
    peripheral.disconnect();
    return;
  } else {
    Serial.println("Subscribed");
    Serial.println("Press reading updated sensor values.");
  }

  digitalWrite(LED_BUILTIN, HIGH);
  while (peripheral.connected()) {
    // while the peripheral is connected

    // check if the value of the simple key characteristic has been updated
    if (zAccelCharacteristic.valueUpdated()) {

      //read float sensor characteristics      
      xAccelCharacteristic.readValue(&xAccel,6);
      yAccelCharacteristic.readValue(&yAccel,6);
      zAccelCharacteristic.readValue(&zAccel,6);


      Serial.println("Read values from lego-G");
      Serial.println(xAccel);
      Serial.println(yAccel);
      Serial.println(zAccel);

      // perform processing and compute the resultant
      resAccel = sqrt(pow(xAccel,2)+pow(yAccel,2)+pow(zAccel,2));
      Serial.println("Resultant Acceleration");
      Serial.println(resAccel);

    }
  }

  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("Peripheral disconnected");
}