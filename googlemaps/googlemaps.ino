
#include <Arduino.h>
#ifdef ARDUINO_ARCH_SAMD
#include <WiFi101.h>
#elif defined ARDUINO_ARCH_ESP8266
#include <ESP8266WiFi.h>
#elif defined ARDUINO_ARCH_ESP32
#include <WiFi.h>
#else
#error Wrong platform
#endif 
long previousMillis = 0;
#include <WifiLocation.h>
#include <Wire.h>  // Wire library - used for I2C communication
int ADXL345 = 0x53; // The ADXL345 sensor I2C address
float X_out, Y_out, Z_out;  // Outputs
float roll,pitch,rollF,pitchF=0;
const char* googleApiKey = "AIzaSyAP9YLSDLSxlcB--h9Y_g9CYctxyuY-DRw";
const char* ssid = "Mikrotik_Koders";
const char* passwd = "1Qw23er4";
const int id = 1;
    
WifiLocation location(googleApiKey);
void setup() {
    Serial.begin(115200);
    // Connect to WPA/WPA2 network
#ifdef ARDUINO_ARCH_ESP32
    WiFi.mode(WIFI_MODE_STA);
#endif
#ifdef ARDUINO_ARCH_ESP8266
    WiFi.mode(WIFI_STA);
#endif
    WiFi.begin(ssid, passwd);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print("Attempting to connect to WPA SSID: ");
        Serial.println(ssid);
        // wait 5 seconds for connection:
        Serial.print("Status = ");
        Serial.println(WiFi.status());
        delay(500);
    }
    Serial.print("Connect");
//-----------------------------------------------------
Wire.begin(); // Initiate the Wire library
  // Set ADXL345 in measuring mode
  Wire.beginTransmission(ADXL345); // Start communicating with the device
  Wire.write(0x2D); // Access/ talk to POWER_CTL Register - 0x2D
  // Enable measurement
  Wire.write(8); // Bit D3 High for measuring enable (8dec -> 0000 1000 binary)
  Wire.endTransmission();
  delay(10);
  //Off-set Calibration
  //X-axis
  Wire.beginTransmission(ADXL345);
  Wire.write(0x1E);
  Wire.write(1);
  Wire.endTransmission();
  delay(10);
  //Y-axis
  Wire.beginTransmission(ADXL345);
  Wire.write(0x1F);
  Wire.write(-2);
  Wire.endTransmission();
  delay(10);
  //Z-axis
  Wire.beginTransmission(ADXL345);
  Wire.write(0x20);
  Wire.write(-9);
  Wire.endTransmission();
  delay(10);
//-----------------------------------------------------

}

void loop() {
  
    unsigned long time = millis();
   
   if (time - previousMillis > 59999){
      location_t loc = location.getGeoFromWiFi();
      /*Serial.println("Location request data");
      Serial.println(location.getSurroundingWiFiJson());
      Serial.println("Latitude: " + String(loc.lat, 7)); // 1
      Serial.println("Longitude: " + String(loc.lon, 7)); // 2
      Serial.println("Accuracy: " + String(loc.accuracy));
      Serial.println("==============================================");*/
      //get запрос здесь
      //char *gps = "GET / HTTP/1.1\r\nHost:\r\"" + char(loc.lat, 7) + "" + char(loc.lon, 7) + "\r\n\r\n"; + id
      //wifi.send((const uint8_t*)hello, strlen(hello));
      previousMillis = time;
   }
   
   Wire.beginTransmission(ADXL345);
   Wire.write(0x32); // Start with register 0x32 (ACCEL_XOUT_H)
   Wire.endTransmission(false);
   Wire.requestFrom(ADXL345, 6, true); // Read 6 registers total, each axis value is stored in 2 registers
   X_out = ( Wire.read() | Wire.read() << 8); // X-axis value
   X_out = X_out / 256; //For a range of +-2g, we need to divide the raw values by 256, according to the datasheet
   Y_out = ( Wire.read() | Wire.read() << 8); // Y-axis value
   Y_out = Y_out / 256;
   Z_out = ( Wire.read() | Wire.read() << 8); // Z-axis value
   Z_out = Z_out / 256;
   // Calculate Roll and Pitch (rotation around X-axis, rotation around Y-axis)
   roll = atan(Y_out / sqrt(pow(X_out, 2) + pow(Z_out, 2))) * 180 / PI;
   pitch = atan(-1 * X_out / sqrt(pow(Y_out, 2) + pow(Z_out, 2))) * 180 / PI;
   // Low-pass filter
   rollF = 0.94 * rollF + 0.06 * roll;
   pitchF = 0.94 * pitchF + 0.06 * pitch;
   //Serial.print(rollF);
   //Serial.print("/");
   //Serial.println(pitchF);
   float rollf_on = rollF + 50;
   float pitchf_on = pitchF + 50;
   if (rollF > rollf_on || pitchF > pitchf_on){
    Serial.print("SOS");
    // get запросы при резком смене дислокации
    //char *gps = "GET / HTTP/1.1\r\nHost:\r\SOS\r\n\r\n"; + id
    //wifi.send((const uint8_t*)hello, strlen(hello));
   }
}
