#include <bluefruit.h>
//#include <U8x8lib.h>
//#include <Wire.h>

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <Wire.h>

#include "NotoSansBold36.h"

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


TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
TFT_eSprite face = TFT_eSprite(&tft);

#define COLOUR_BG   TFT_NAVY
#define COLOUR_HEAD TFT_GOLD
#define COLOUR_HIGHLIGHTS TFT_BLACK
#define COLOUR_TEETH TFT_WHITE
#define COLOUR_MOUTH TFT_BLACK
#define COLOUR_BROWS TFT_BLACK
#define COLOUR_TONGUE TFT_RED
#define COLOUR_CREASE TFT_BROWN
#define COLOUR_EYE TFT_BLACK
#define COLOUR_PUPIL TFT_WHITE
#define COLOUR_SWEAT TFT_BLUE


#define COLOUR_TEXT TFT_GOLD


#define SCREEN_RADIUS       230.0f / 2.0f // Clock face radius (float type)
#define FACE_W SCREEN_RADIUS * 2 + 1
#define FACE_H SCREEN_RADIUS * 2 + 1

void setup() {
  Serial.begin(9600);
  while ( !Serial ) delay(10);   // for nrf52840 with native usb

  // Boot screen first so it isn't trying to communicate at the same time.
  Serial.println("Booting Display...");

  // Initialise the screen
  tft.init();

  // Ideally set orientation for good viewing angle range because
  // the anti-aliasing effectiveness varies with screen viewing angle
  // Usually this is when screen ribbon connector is at the bottom
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  // Create the clock face sprite
  //face.setColorDepth(8); // 8 bit will work, but reduces effectiveness of anti-aliasing
  face.createSprite(FACE_W, FACE_H);

  // Only 1 font used in the sprite, so can remain loaded
  face.loadFont(NotoSansBold36);

  Wire.begin();

  resetScreen();
  drawMinifigHead();
  drawHappyFace();
  forceRefresh();
  delay(500);
  
  resetScreen();
  drawMinifigHead();
  drawSweatingFace();
  forceRefresh();
  delay(500);
  
  resetScreen();
  drawMinifigHead();
  drawWorriedFace();
  forceRefresh();
  delay(500);
  
  resetScreen();
  drawMinifigHead();
  drawCryingFace();
  forceRefresh();
  delay(500);
  
  resetScreen();
  drawMinifigHead();
  drawPukingFace();
  forceRefresh();
  delay(500);
  
  resetScreen();
  drawMinifigHead();
  drawDeadFace();
  forceRefresh();
  delay(500);

  resetScreen();
  drawMinifigHead();
  drawHappyFace();
  forceRefresh();
  delay(500);


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


  resetScreen();
  drawMinifigHead();

  if(resAccel <= 1.5){
    drawHappyFace();
  }
  else if (resAccel <= 2.5){
    drawSweatingFace();    
  }
  else if (resAccel <= 3.5){
    drawWorriedFace();
  }
  else if (resAccel <= 4.5){
    drawCryingFace();
  }
  else if (resAccel <= 5.5){
    drawPukingFace();
  }
  else{
    drawDeadFace();
  }
  writeGText();
  forceRefresh();
  delay(500);

}



void drawThickLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color, int32_t t)
{
  // Add thickness wrapper to the default Line library.
  for (int i; i<t; i++){
    face.drawLine(x0, y0+i, x1, y1+i, color);
  }
}

void resetScreen()
{
 // The face is completely redrawn - this can be done quickly
  face.fillSprite(TFT_BLACK);

  // Draw the background circle
  face.fillSmoothCircle( SCREEN_RADIUS, SCREEN_RADIUS, SCREEN_RADIUS, COLOUR_BG );
}


void drawMinifigHead()
{
  // *** DRAW MINIFIG HEAD ***//
  face.fillRect( 94-5, 32, 52, 19, COLOUR_HEAD );
  face.fillRoundRect( 65-5, 51, 110, 90, 18, COLOUR_HEAD );
  face.fillRect( 77-5, 141, 86, 11, COLOUR_HEAD );
}

void forceRefresh()
{
  face.pushSprite(5, 5, TFT_TRANSPARENT);
}


void drawCryingFace()
{
  // *** DRAW CRYING FACE ***//
  //eyes
  drawThickLine(99-10, 86, 104-10, 83, COLOUR_EYE, 8);
  drawThickLine(104-10, 84, 108-10, 85, COLOUR_EYE, 8);
  drawThickLine(108-10, 85, 110-10, 87, COLOUR_EYE, 8);
  
  drawThickLine(135, 88, 139, 86, COLOUR_EYE, 8);
  drawThickLine(139, 86, 142, 86, COLOUR_EYE, 8);
  drawThickLine(142, 86, 146, 89, COLOUR_EYE, 8);

  //eyebrows
  drawThickLine(95-10, 79-5, 107-10, 79-5, COLOUR_BROWS, 8);
  drawThickLine(107-10, 79-5, 118-10, 71-5, COLOUR_BROWS, 8);
  
  drawThickLine(131, 70-5, 140, 79-5, COLOUR_BROWS, 8);
  drawThickLine(140, 79-5, 150, 83-5, COLOUR_BROWS, 8);

  //mouth
  face.fillTriangle(112, 118, 93, 97, 85, 110, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 85, 110, 85, 118, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 85, 118, 91, 125, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 91, 125, 100, 124, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 100, 124, 109, 122, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 109, 122, 117, 124, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 117, 124, 123, 127, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 123, 127, 130, 128, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 130, 128, 138, 128, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 138, 128, 142, 121, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 142, 121, 143, 111, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 143, 111, 138, 102, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 138, 102, 131, 97, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 131, 97, 119, 102, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 119, 102, 110, 102, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 110, 102, 101, 99, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 101, 99, 93, 97, COLOUR_MOUTH);



  //tongue
  drawThickLine(106-5, 117, 113-5, 112, COLOUR_TONGUE, 8);
  drawThickLine(113-5, 112, 119-5, 116, COLOUR_TONGUE, 8);
  drawThickLine(119-5, 116, 128-5, 113, COLOUR_TONGUE, 8);
  drawThickLine(128-5, 113, 133-5, 119, COLOUR_TONGUE, 8);

  //teeth
  drawThickLine(102-7, 98+2, 115-7, 101+2, COLOUR_TEETH, 8);
  drawThickLine(115-7, 101+2, 129-7, 102+2, COLOUR_TEETH, 8);
  drawThickLine(129-7, 102+2, 139-7, 99+2, COLOUR_TEETH, 8);
}

void drawHappyFace()
{
 // *** DRAW HAPPY FACE ***//
  //eyes
  face.fillSmoothCircle( 92, 91, 11, COLOUR_EYE );
  face.fillSmoothCircle( 138, 91, 11, COLOUR_EYE );
  face.fillSmoothCircle( 92, 88, 4, COLOUR_PUPIL );
  face.fillSmoothCircle( 138, 88, 4, COLOUR_PUPIL );

  //eyebrows
  drawThickLine(92-10, 74, 98-10, 69, COLOUR_BROWS, 8);
  drawThickLine(98-10, 69, 105-10, 68, COLOUR_BROWS, 8);
  drawThickLine(105-10, 68, 115-10, 70, COLOUR_BROWS, 8);
  
  drawThickLine(129, 71, 137, 67, COLOUR_BROWS, 8);
  drawThickLine(137, 67, 144, 69, COLOUR_BROWS, 8);
  drawThickLine(144, 69, 149, 73, COLOUR_BROWS, 8);

  //eye creases
  drawThickLine(87-15, 90, 92-15, 93, COLOUR_CREASE, 2);
  drawThickLine(89-15, 96, 95-15, 96, COLOUR_CREASE, 2);
  
  drawThickLine(147+6, 92, 153+6, 91, COLOUR_CREASE, 2);
  drawThickLine(146+6, 95, 151+6, 96, COLOUR_CREASE, 2);

  //chin crease
  drawThickLine(111-5, 132, 118-5, 133, COLOUR_CREASE, 4);
  drawThickLine(118-5, 133, 125-5, 133, COLOUR_CREASE, 4);
  drawThickLine(125-5, 133, 131-5, 132, COLOUR_CREASE, 4);

  //mouth
  face.fillTriangle(112, 118, 89, 105, 89, 118, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 89, 118, 100, 126, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 100, 126, 112, 129, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 112, 129, 125, 127, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 125, 127, 135, 121, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 135, 121, 137, 117, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 137, 117, 137, 105, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 137, 105, 132, 109, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 132, 109, 122, 113, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 122, 113, 106, 113, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 106, 113, 96, 110, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 96, 110, 89, 105, COLOUR_MOUTH);

  //tongue
  drawThickLine(111-5, 122, 131-5, 122, COLOUR_TONGUE, 8);

  //teeth
  drawThickLine(99-7, 110, 110-7, 115, COLOUR_TEETH, 8);
  drawThickLine(110-7, 115, 121-7, 116, COLOUR_TEETH, 8);
  drawThickLine(121-7, 116, 131-7, 115, COLOUR_TEETH, 8);
  drawThickLine(131-7, 115, 142-7, 110, COLOUR_TEETH, 8);
}


void drawSweatingFace()
{
  // *** DRAW SWEATY FACE ***//
  //eyes
  face.fillSmoothCircle( 92, 91, 11, COLOUR_EYE );
  face.fillSmoothCircle( 138, 91, 11, COLOUR_EYE );
  face.fillSmoothCircle( 92, 88, 4, COLOUR_PUPIL );
  face.fillSmoothCircle( 138, 88, 4, COLOUR_PUPIL );

  //eyebrows
  drawThickLine(92-10, 72, 103-10, 73, COLOUR_BROWS, 8);
  drawThickLine(103-10, 73, 110-10, 72, COLOUR_BROWS, 8);
  drawThickLine(110-10, 72, 115-10, 69, COLOUR_BROWS, 8);
  
  drawThickLine(129, 67, 139, 64, COLOUR_BROWS, 8);
  drawThickLine(139, 64, 149, 69, COLOUR_BROWS, 8);
  drawThickLine(149, 69, 154, 74, COLOUR_BROWS, 8);

  // mouth
  drawThickLine(101-5, 116, 108-5, 111, COLOUR_MOUTH, 8);
  drawThickLine(108-5, 111, 117-5, 115, COLOUR_MOUTH, 8);
  drawThickLine(117-5, 115, 123-5, 113, COLOUR_MOUTH, 8);
  drawThickLine(123-5, 113, 131-5, 116, COLOUR_MOUTH, 8);
  drawThickLine(131-5, 116, 138-5, 114, COLOUR_MOUTH, 8);

  //sweat
  int drip_x = 70;
  int drip_y = 64;
  
  face.fillTriangle(drip_x, drip_y, drip_x-7, drip_y+1+20, drip_x-6, drip_y+5+20, COLOUR_SWEAT);
  face.fillTriangle(drip_x, drip_y, drip_x-6, drip_y+5+20, drip_x-4, drip_y+8+20, COLOUR_SWEAT);
  face.fillTriangle(drip_x, drip_y, drip_x-4, drip_y+8+20, drip_x-1, drip_y+9+20, COLOUR_SWEAT);
  face.fillTriangle(drip_x, drip_y, drip_x-1, drip_y+9+20, drip_x+2, drip_y+8+20, COLOUR_SWEAT);
  face.fillTriangle(drip_x, drip_y, drip_x+2, drip_y+8+20, drip_x+5, drip_y+6+20, COLOUR_SWEAT);
  face.fillTriangle(drip_x, drip_y, drip_x+5, drip_y+6+20, drip_x+6, drip_y+4+20, COLOUR_SWEAT);
  face.fillTriangle(drip_x, drip_y, drip_x+6, drip_y+4+20, drip_x+7, drip_y+2+20, COLOUR_SWEAT);
  face.fillTriangle(drip_x, drip_y, drip_x+7, drip_y+2+20, drip_x+6, drip_y-1+20, COLOUR_SWEAT);
}


void drawPukingFace()
{ 
  // *** DRAW PUKING FACE ***//
  //eyes
  face.fillSmoothCircle( 92, 91, 11, COLOUR_EYE );
  face.fillSmoothCircle( 138, 91, 11, COLOUR_EYE );
  face.fillSmoothCircle( 92, 88, 4, COLOUR_PUPIL );
  face.fillSmoothCircle( 138, 88, 4, COLOUR_PUPIL );
  
  //eye creases
  drawThickLine(95-15, 95, 112-15, 103, COLOUR_CREASE, 4);
  drawThickLine(134, 102, 152, 93, COLOUR_CREASE, 4);

  //eyebrows
  drawThickLine(92-10, 74, 98-10, 69, COLOUR_BROWS, 8);
  drawThickLine(98-10, 69, 105-10, 68, COLOUR_BROWS, 8);
  drawThickLine(105-10, 68, 115-10, 70, COLOUR_BROWS, 8);
  
  drawThickLine(129, 71, 137, 67, COLOUR_BROWS, 8);
  drawThickLine(137, 67, 144, 69, COLOUR_BROWS, 8);
  drawThickLine(144, 69, 149, 73, COLOUR_BROWS, 8);

  //eyebrow creases
  drawThickLine(116-5, 61, 118-5, 66, COLOUR_CREASE, 4);
  drawThickLine(118-5, 66, 118-5, 71, COLOUR_CREASE, 4);

  drawThickLine(124-5, 61, 123-5, 67, COLOUR_CREASE, 4);
  drawThickLine(123-5, 67, 124-5, 70, COLOUR_CREASE, 4);

  //cheek creases
  drawThickLine(82-5, 93, 76-5, 101, COLOUR_CREASE, 4);
  drawThickLine(76-5, 101, 77-5, 113, COLOUR_CREASE, 4);
  drawThickLine(77-5, 113, 83-5, 122, COLOUR_CREASE, 4);

  drawThickLine(157, 93, 162, 103, COLOUR_CREASE, 4);
  drawThickLine(162, 103, 162, 110, COLOUR_CREASE, 4);
  drawThickLine(162, 110, 157, 120, COLOUR_CREASE, 4);

  //chin crease
  drawThickLine(116-5, 124+5, 121-5, 125+5, COLOUR_CREASE, 4);
  drawThickLine(121-5, 125+5, 125-5, 124+5, COLOUR_CREASE, 4);

  //mouth
  drawThickLine(116-5, 110, 118-5, 113, COLOUR_MOUTH, 4);
  drawThickLine(118-5, 113, 115-5, 119, COLOUR_MOUTH, 4);
  drawThickLine(118-5, 113, 126-5, 113, COLOUR_MOUTH, 4);
  drawThickLine(129-5, 110, 126-5, 113, COLOUR_MOUTH, 4);
  drawThickLine(126-5, 113, 129-5, 120, COLOUR_MOUTH, 4);
}


void drawWorriedFace()
{
    // *** DRAW WORRIED FACE ***//
  //eyes
  face.fillSmoothCircle( 92, 91, 11, COLOUR_EYE );
  face.fillSmoothCircle( 138, 91, 11, COLOUR_EYE );
  face.fillSmoothCircle( 92, 88, 4, COLOUR_PUPIL );
  face.fillSmoothCircle( 138, 88, 4, COLOUR_PUPIL );

  //eyebrows
  drawThickLine(88-15, 74, 94-15, 66, COLOUR_BROWS, 8);
  drawThickLine(94-15, 66, 103-15, 62, COLOUR_BROWS, 8);
  drawThickLine(103-15, 62, 111-15, 60, COLOUR_BROWS, 8);

  drawThickLine(130, 55, 140, 57, COLOUR_BROWS, 8);
  drawThickLine(140, 57, 146, 60, COLOUR_BROWS, 8);
  drawThickLine(146, 60, 154,67, COLOUR_BROWS, 8);

  //mouth
  face.fillTriangle(112, 118, 105, 105, 88, 110, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 88, 110, 84, 117, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 84, 117, 82, 128, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 82, 128, 88, 136, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 88, 136, 98, 136, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 98, 136, 107, 132, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 107, 132, 117, 131, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 117, 131, 137, 135, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 137, 135, 143, 132, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 143, 132, 143, 117, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 143, 117, 137, 110, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 137, 110, 123, 105, COLOUR_MOUTH);
  face.fillTriangle(112, 118, 123, 105, 105, 105, COLOUR_MOUTH);

  //teeth
  drawThickLine(103-8, 111+2, 139-10, 111+2, COLOUR_TEETH, 8);

  drawThickLine(100-8, 130+2, 116-8, 125+2, COLOUR_TEETH, 6);  
  drawThickLine(116-8, 125+2, 130-8, 125+2, COLOUR_TEETH, 6);
  drawThickLine(130-8, 125+2, 140-8, 129+2, COLOUR_TEETH, 6);
}


void drawDeadFace()
{
  // *** DRAW DEAD FACE ***//
  //eyes
  drawThickLine(82, 81, 102, 101, COLOUR_EYE, 8);
  drawThickLine(82, 101, 102, 81, COLOUR_EYE, 8);
  drawThickLine(128, 81, 148, 101, COLOUR_EYE, 8);
  drawThickLine(128, 101, 148, 81, COLOUR_EYE, 8);

  //mouth
  drawThickLine(92, 130, 138, 120, COLOUR_EYE, 5);

  //tongue
  face.fillTriangle(130, 111+10, 123, 112+20, 124, 116+20, COLOUR_TONGUE);
  face.fillTriangle(130, 111+10, 124, 116+20, 126, 119+20, COLOUR_TONGUE);
  face.fillTriangle(130, 111+10, 126, 119+20, 129, 120+20, COLOUR_TONGUE);
  face.fillTriangle(130, 111+10, 129, 120+20, 132, 119+20, COLOUR_TONGUE);
  face.fillTriangle(130, 111+10, 132, 119+20, 135, 117+20, COLOUR_TONGUE);
  face.fillTriangle(130, 111+10, 135, 117+20, 136, 115+20, COLOUR_TONGUE);
  face.fillTriangle(130, 111+10, 136, 115+20, 137, 113+20, COLOUR_TONGUE);
  face.fillTriangle(130, 111+10, 137, 113+20, 136, 110+20, COLOUR_TONGUE);

}

void writeGText()
{
  // Add text (could be digital time...)
  face.setTextColor(COLOUR_TEXT);
  face.setTextDatum(BC_DATUM);
  face.drawString(String(resAccel,1) + "G", 120, 208);
}

