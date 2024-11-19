#include <bluefruit.h>
//#include <U8x8lib.h>
//#include <Wire.h>

// THIS IS THE CENTRAL (always scanning)
// https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/master/libraries/Bluefruit52Lib/examples/Central/central_custom_hrm/central_custom_hrm.ino

// initialize readings
float xAccel = 0;
float yAccel = 0;
float zAccel = 0;
float resAccel = 0; 

// Initialize bluetooth service
BLEClientService legoGService(0x19B1); // Bluetooth® Low Energy LED Service

// Bluetooth® Low Energy data Characteristic - custom 128-bit UUID, read and writable by central
BLEClientCharacteristic xAccelerationCharacteristic(0x0695);
BLEClientCharacteristic yAccelerationCharacteristic(0x1c12);
BLEClientCharacteristic zAccelerationCharacteristic(0x6102);


void setup() {
  Serial.begin(9600);
  while ( !Serial ) delay(10);   // for nrf52840 with native usb

  Serial.println("Lego IMU Central");
  Serial.println("--------------------------------------\n");

  // Initialize Bluefruit with maximum connections as Peripheral = 0, Central = 1
  // SRAM usage required by SoftDevice will increase dramatically with number of connections
  Bluefruit.begin(0, 1);

  Bluefruit.setName("Lego-G Central");

  // Initialize HRM client
  legoGService.begin();

  // Initialize client characteristics of first two IMU measurements.
  // Note: Client Char will be added to the last service that is begin()ed.
  xAccelerationCharacteristic.begin();
  yAccelerationCharacteristic.begin();

  // set up callback for receiving final measurement
  zAccelerationCharacteristic.setNotifyCallback(imu_notify_callback);
  zAccelerationCharacteristic.begin();

  // Increase Blink rate to different from PrPh advertising mode
  Bluefruit.setConnLedInterval(250);

  // Callbacks for Central
  Bluefruit.Central.setDisconnectCallback(disconnect_callback);
  Bluefruit.Central.setConnectCallback(connect_callback);

  /* Start Central Scanning
   * - Enable auto scan if disconnected
   * - Interval = 100 ms, window = 80 ms
   * - Don't use active scan
   * - Filter only accept HRM service
   * - Start(timeout) with timeout = 0 will scan forever (until connected)
   */
  Bluefruit.Scanner.setRxCallback(scan_callback);
  Bluefruit.Scanner.restartOnDisconnect(true);
  Bluefruit.Scanner.setInterval(160, 80); // in unit of 0.625 ms
  Bluefruit.Scanner.filterUuid(legoGService.uuid);
  Bluefruit.Scanner.useActiveScan(false);
  Bluefruit.Scanner.start(0);                   // // 0 = Don't stop scanning after n seconds


}

void loop() {
  // do nothing in loop, everything is handled by callbacks

}

/**
 * Callback invoked when scanner pick up an advertising data
 * @param report Structural advertising data
 */
void scan_callback(ble_gap_evt_adv_report_t* report)
{
  // Since we configure the scanner with filterUuid()
  // Scan callback only invoked for device with hrm service advertised
  // Connect to device with HRM service in advertising
  Bluefruit.Central.connect(report);
}

/**
 * Callback invoked when an connection is established
 * @param conn_handle
 */
void connect_callback(uint16_t conn_handle)
{
  Serial.println("Connected");
  Serial.print("Discovering IMU Service ... ");

  // If IMU is not found, disconnect and return
  if ( !legoGService.discover(conn_handle) )
  {
    Serial.println("Found NONE");

    // disconnect since we couldn't find HRM service
    Bluefruit.disconnect(conn_handle);

    return;
  }

  // Once HRM service is found, we continue to discover its characteristic
  Serial.println("Found it");
  
  Serial.print("Discovering Measurement characteristic ... ");
  if ( !xAccelerationCharacteristic.discover() )
  {
    // Measurement chr is mandatory, if it is not found (valid), then disconnect
    Serial.println("not found !!!");  
    Serial.println("Measurement characteristic X is mandatory but not found");
    Bluefruit.disconnect(conn_handle);
    return;
  }
  if ( !yAccelerationCharacteristic.discover() )
  {
    // Measurement chr is mandatory, if it is not found (valid), then disconnect
    Serial.println("not found !!!");  
    Serial.println("Measurement characteristic Y is mandatory but not found");
    Bluefruit.disconnect(conn_handle);
    return;
  }
  if ( !zAccelerationCharacteristic.discover() )
  {
    // Measurement chr is mandatory, if it is not found (valid), then disconnect
    Serial.println("not found !!!");  
    Serial.println("Measurement characteristic Z is mandatory but not found");
    Bluefruit.disconnect(conn_handle);
    return;
  }
  Serial.println("Found it");

  // Reaching here means we are ready to go, let's enable notification on measurement chr
  if ( zAccelerationCharacteristic.enableNotify() )
  {
    Serial.println("Ready to receive IMU Measurement value");
  }else
  {
    Serial.println("Couldn't enable notify for IMU Measurement. Increase DEBUG LEVEL for troubleshooting");
  }
  digitalWrite(LED_BUILTIN, HIGH);
}

/**
 * Callback invoked when a connection is dropped
 * @param conn_handle
 * @param reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
 */
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.print("Disconnected, reason = 0x"); Serial.println(reason, HEX);
  digitalWrite(LED_BUILTIN, LOW);
}


/**
 * Hooked callback that triggered when a measurement value is sent from peripheral
 * @param chr   Pointer client characteristic that even occurred,
 *              in this example it should be hrmc
 * @param data  Pointer to received data
 * @param len   Length of received data
 */
void imu_notify_callback(BLEClientCharacteristic* chr, uint8_t* data, uint16_t len)
{
  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.characteristic.heart_rate_measurement.xml
  // Measurement contains of control byte0 and measurement (8 or 16 bit) + optional field
  // if byte0's bit0 is 0 --> measurement is 8 bit, otherwise 16 bit.
  Serial.println("IMU Notify Callback");


  xAccelerationCharacteristic.read(&xAccel, 4);
  yAccelerationCharacteristic.read(&yAccel, 4);
  zAccelerationCharacteristic.read(&zAccel, 4);


  resAccel = sqrt(pow(xAccel,2)+pow(yAccel,2)+pow(zAccel,2));
  Serial.print("IMU Measurement: x=");
  Serial.print(xAccel);
  Serial.print(" y=");
  Serial.print(yAccel);
  Serial.print(" y=");
  Serial.print(zAccel);
  Serial.print(" res=");
  Serial.print(resAccel);


}

