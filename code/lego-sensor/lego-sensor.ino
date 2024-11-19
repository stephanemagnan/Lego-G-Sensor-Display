#include <bluefruit.h>
#include "LSM6DS3.h"
#include "Wire.h"

// THIS IS THE PERIPHERAL (sleeping then sending)
// IMU code from: Examples > Seeed Arduino LSM6DS3 > HighLevelExample.ino
// IMU reference: https://wiki.seeedstudio.com/XIAO-BLE-Sense-IMU-Usage/
// Bluetooth code from: https://wiki.seeedstudio.com/XIAO-BLE-Sense-Bluetooth-Usage/
// Bluefruit https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/master/libraries/Bluefruit52Lib/examples/Peripheral/custom_hrm/custom_hrm.ino



// Initialize definitions for inertial measurement
LSM6DS3 myIMU(I2C_MODE, 0x6A);    //I2C device address 0x6A
float xAccel, yAccel, zAccel;

// Initialize bluetooth service
BLEService legoGService(0x19B1); // Bluetooth® Low Energy LED Service

// Bluetooth® Low Energy data Characteristic - custom 128-bit UUID, read and writable by central
BLECharacteristic xAccelerationCharacteristic(0x0695);
BLECharacteristic yAccelerationCharacteristic(0x1c12);
BLECharacteristic zAccelerationCharacteristic(0x6102);

BLEDis bledis;    // DIS (Device Information Service) helper class instance
BLEBas blebas;    // BAS (Battery Service) helper class instance

void setup() {
  Serial.begin(9600);
  while (!Serial); delay(10);

    if (myIMU.begin() != 0) {
        Serial.println("IMU error");
    } else {
        Serial.println("IMU OK!");
    }

  // Initialise the Bluefruit module
  Serial.println("Initialise the Bluefruit nRF52 module");
  Bluefruit.begin();

  // Set the connect/disconnect callback handlers
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  // Configure and Start the Device Information Service
  Serial.println("Configuring the Device Information Service");
  bledis.setManufacturer("XIAO");
  bledis.setModel("Seeed nRF52 Sense");
  bledis.begin();

  // Start the BLE Battery Service and set it to 100%
  Serial.println("Configuring the Battery Service");
  blebas.begin();
  blebas.write(100);

  // Setup the IMU service using
  // BLEService and BLECharacteristic classes
  Serial.println("Configuring the IMU Service");
  setupIMU();

  // Setup the advertising packet(s)
  Serial.println("Setting up the advertising payload(s)");
  startAdv();

  Serial.println("Ready Player One!!!");
  Serial.println("\nAdvertising");


// read sensor values  
  Serial.println("readingAccel");
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

}

void loop() {
  Serial.println("In loop");
  if ( Bluefruit.connected() ) {
    //connected
    Serial.print("Bluefruit connected!");


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

    // Note: We use .notify instead of .write!
    // If it is connected but CCCD is not enabled
    // The characteristic's value is still updated although notification is not sent
    xAccelerationCharacteristic.writeFloat(xAccel);
    yAccelerationCharacteristic.writeFloat(yAccel);

    if ( zAccelerationCharacteristic.notify32(zAccel)){
      Serial.println("IMU values updated");
    }else{
      Serial.println("ERROR: Notify not set in the CCCD or not connected!");
    }

  }

  // Only send update once per second
  Serial.println("Waiting 1 second");
  delay(1000);



}

void startAdv(void)
{
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();

  // Include LegoG Service UUID
  Bluefruit.Advertising.addService(legoGService);

  // Include Name
  Bluefruit.Advertising.addName();
  
  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

void setupIMU(void)
{
  // Configure the Heart Rate Monitor service
  // See: https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.service.heart_rate.xml
  // Supported Characteristics:
  // Name                         UUID    Requirement Properties
  // ---------------------------- ------  ----------- ----------
  // Heart Rate Measurement       0x2A37  Mandatory   Notify
  // Body Sensor Location         0x2A38  Optional    Read
  // Heart Rate Control Point     0x2A39  Conditional Write       <-- Not used here
  legoGService.begin();

  // Note: You must call .begin() on the BLEService before calling .begin() on
  // any characteristic(s) within that service definition.. Calling .begin() on
  // a BLECharacteristic will cause it to be added to the last BLEService that
  // was 'begin()'ed!

  // Configure the IMU characteristics
  
  xAccelerationCharacteristic.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE);
  xAccelerationCharacteristic.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  xAccelerationCharacteristic.setFixedLen(4);
  //xAccelerationCharacteristic.setCccdWriteCallback(cccd_callback);  // Optionally capture CCCD updates
  xAccelerationCharacteristic.begin();
  uint8_t xaccdata[4] = { 0x0, 0x0, 0x0, 0x0}; // Set the characteristic to use 8-bit values, with the sensor connected and detected
  xAccelerationCharacteristic.write(xaccdata, 4);

  yAccelerationCharacteristic.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE);
  yAccelerationCharacteristic.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  yAccelerationCharacteristic.setFixedLen(4);
  //yAccelerationCharacteristic.setCccdWriteCallback(cccd_callback);  // Optionally capture CCCD updates
  yAccelerationCharacteristic.begin();
  uint8_t yaccdata[4] = { 0x0, 0x0, 0x0, 0x0 }; // Set the characteristic to use 8-bit values, with the sensor connected and detected
  yAccelerationCharacteristic.write(yaccdata, 4);
 
  zAccelerationCharacteristic.setProperties(CHR_PROPS_READ | CHR_PROPS_NOTIFY);
  zAccelerationCharacteristic.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  zAccelerationCharacteristic.setFixedLen(4);
  //zAccelerationCharacteristic.setCccdWriteCallback(cccd_callback);  // Optionally capture CCCD updates
  zAccelerationCharacteristic.begin();
  uint8_t zaccdata[4] = { 0x0, 0x0, 0x0, 0x0 }; // Set the characteristic to use 8-bit values, with the sensor connected and detected
  zAccelerationCharacteristic.write(zaccdata, 4);
}

void connect_callback(uint16_t conn_handle)
{
  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);

  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));

  Serial.print("Connected to ");
  Serial.println(central_name);
}

/**
 * Callback invoked when a connection is dropped
 * @param conn_handle connection where this event happens
 * @param reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
 */
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.print("Disconnected, reason = 0x"); Serial.println(reason, HEX);
  Serial.println("Advertising!");
}

void cccd_callback(uint16_t conn_hdl, BLECharacteristic* chr, uint16_t cccd_value)
{
    // Display the raw request packet
    Serial.print("CCCD Updated: ");
    //Serial.printBuffer(request->data, request->len);
    Serial.print(cccd_value);
    Serial.println("");

    // Check the characteristic this CCCD update is associated with in case
    // this handler is used for multiple CCCD records.
    // if (chr->uuid == xAccelerationCharacteristic.uuid) {
    //     if (chr->notifyEnabled(conn_hdl)) {
    //         Serial.println("xAccel 'Notify' enabled");
    //     } else {
    //         Serial.println("xAccel 'Notify' disabled");
    //     }
    // }
    // if (chr->uuid == yAccelerationCharacteristic.uuid) {
    //     if (chr->notifyEnabled(conn_hdl)) {
    //         Serial.println("yAccel 'Notify' enabled");
    //     } else {
    //         Serial.println("yAccel 'Notify' disabled");
    //     }
    // }
    if (chr->uuid == zAccelerationCharacteristic.uuid) {
        if (chr->notifyEnabled(conn_hdl)) {
            Serial.println("zAccel 'Notify' enabled");
        } else {
            Serial.println("zAccel 'Notify' disabled");
        }
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