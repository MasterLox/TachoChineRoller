#include <Wire.h>
#include <TFT_eSPI.h>
#include <MPU6050.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>

// Pins for GPS module
static const int RXPin = 16, TXPin = 17;
static const uint32_t GPSBaud = 115200;

TFT_eSPI tft = TFT_eSPI(); // Initialize the display
MPU6050 mpu; // Create an instance of the MPU6050

// GPS Module
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);

#define SCREEN_CENTER_X 120 // Center X of the screen
#define SCREEN_CENTER_Y 120 // Center Y of the screen

int rpm = 0;               // Current RPM value
int maxRPM = 8000;         // Maximum RPM value
float temperature = 22.5;  // Initial temperature value (placeholder)
String currentTime = "12:34"; // Example current time
int satellites = 0;        // Satellitenanzahl
double latitude = 0.0, longitude = 0.0; // GPS coordinates
String speed = "999";         // GPS speed in km/h
String lastTime = "-----";

// Store previous latitude, longitude, and time
double lastLatitude = 0.0, lastLongitude = 0.0;
unsigned long lastTimeMillis = 0;

void setup() {
  // Initialize Serial Monitor for debugging
  Serial.begin(115200);

  // Initialize the display
  tft.init();
  tft.setRotation(0); // Set display orientation
  tft.fillScreen(TFT_BLACK);
  
  // Show boot screen
  showBootScreen();
  
  // Initialize the MPU6050 with SDA on D21 and SCL on D22
  Wire.begin(22, 21); // Use SDA on D21, SCL on D22
  mpu.initialize(); // Initialize MPU6050 without passing any arguments

  // Check if the MPU6050 is connected
  if (mpu.testConnection()) {
    Serial.println("MPU6050 connected successfully!");
  } else {
    Serial.println("MPU6050 connection failed!");
  }

  // Initialize GPS module
  ss.begin(GPSBaud);

  // Wait for at least 3 satellites before proceeding
  int i = 0;
  while (gps.location.isValid()&& i < 500) {
    i++;
    delay(500); // Wait before checking again
  }
  
  // Once we have at least 3 satellites, clear the boot screen
  tft.fillScreen(TFT_BLACK);
  drawStaticElements(); // Draw static elements of the interface
}

void loop() {
  rpm = 0000;
  while (ss.available() > 0)
    if (gps.encode(ss.read())){
        latitude = gps.location.lat();
        longitude = gps.location.lng();
        speed = gps.speed.kmph();
        currentTime = getTime(gps.date.year(), gps.date.month(), gps.date.day(), gps.time.hour(), gps.time.minute());
        satellites = gps.satellites.value();
    }
  temperature = getTemperatureFromMPU6050();
  drawDynamicElements(rpm, satellites);
}
String getTime(int pyear, int pmonth, int pday, int phour, int pminute){

  // Extract GPS date and time
  int year = pyear;
  int month = pmonth;
  int day = pday;
  int hour = phour;
  int minute = pminute;

  // Adjust UTC time to Central European Time (CET)
  // CET = UTC+1 (Winter), CEST = UTC+2 (Summer)
  hour += 1; // Add base offset for CET

  // Determine if it's daylight saving time (DST)
  bool isDST = false;
  if (month > 3 && month < 10) // April to September: DST is active
  {
    isDST = true;
  }
  else if (month == 3 || month == 10) // March or October: check transition days
  {
    // Find last Sunday of March or October
    int lastSunday = day - ((gps.date.day() + 7 - 1) % 7); // 1 = Sunday
    if (month == 3 && day >= lastSunday) isDST = true; // DST starts on the last Sunday of March
    if (month == 10 && day < lastSunday) isDST = true; // DST ends on the last Sunday of October
  }

  if (isDST)
  {
    hour += 1; // Add an additional hour for DST
  }

  // Handle hour overflow (e.g., 24:00 -> 00:00 of the next day)
  if (hour >= 24)
  {
    hour -= 24;
    // Increment date (simple implementation, no month overflow handling for brevity)
    day++;
  }

  // Format time as HH:MM string
  char timeStr[6]; // Buffer for "HH:MM"
  snprintf(timeStr, sizeof(timeStr), "%02d:%02d", hour, minute);

  return String(timeStr);
}
void showBootScreen() {
  // Display boot screen until GPS has found at least 3 satellites
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(3);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Initializing", SCREEN_CENTER_X, SCREEN_CENTER_Y - 40);
  tft.setTextSize(2);
  tft.drawString("Waiting", SCREEN_CENTER_X, SCREEN_CENTER_Y + 20);
  tft.drawString("for GPS signal...", SCREEN_CENTER_X, SCREEN_CENTER_Y + 40);
}

void drawStaticElements() {
  // Clear the screen
  tft.fillScreen(TFT_BLACK);

  // Draw center circle to highlight the speed
  tft.fillCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, 10, TFT_WHITE);
}

void drawDynamicElements(int rpm, int satellites) {
  // Clear the dynamic areas
  //tft.fillRect(20, 40, 200, 160, TFT_BLACK); // Clear the area for RPM, speed, and additional info

  // Display RPM above speed
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(3); // Smaller text for RPM
  tft.drawString(String(rpm) + " RPM", SCREEN_CENTER_X, SCREEN_CENTER_Y - 40);

  // Display speed in the center (from GPS calculation)
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(5); // Larger text for speed
  tft.drawString(speed, SCREEN_CENTER_X, SCREEN_CENTER_Y);

  // Display time and temperature below speed (from GPS)
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(2); // Smaller text for time and temperature
  tft.drawString(currentTime + "  |  " + String(temperature, 1) + "C", SCREEN_CENTER_X, SCREEN_CENTER_Y + 40);

  // Display satellite count below time and temperature
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(2); // Smaller text for satellites
  tft.drawString("Satellites: " + String(satellites), SCREEN_CENTER_X, SCREEN_CENTER_Y + 70);
}

// Function to get temperature from the MPU6050 sensor
float getTemperatureFromMPU6050() {
  // The MPU6050 temperature is in degrees Celsius but needs to be scaled
  float temp = mpu.getTemperature() / 340.00 + 36.53; // Convert to Celsius
  return temp;
}
