/*
 * eScoots_Brain_SAMD51_v2
 * Arduino Code for HighJinx's eScooter
 * used on Adafruit Feather M4 Express
 * and 2.13 Monochrome eInk FeatherWing.
*/


#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include "Adafruit_EPD.h"
#include <Adafruit_NeoPixel.h>
#include "RTClib.h"
#include <Sabertooth.h>

Sabertooth ST(128);

#if defined(ARDUINO_SAMD_FEATHER_M0) || defined(ARDUINO_FEATHER_M4)
  #define SD_CS       5
  #define SRAM_CS     6
  #define EPD_CS      9
  #define EPD_DC      10  
#endif

#define EPD_RESET   -1 // can set to -1 and share with microcontroller Reset!
#define EPD_BUSY    -1 // can set to -1 to not use a pin (will wait a fixed delay)

Adafruit_SSD1675 epd(250, 122, EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY);

#define BUFFPIXEL 20

/* RTC Info. */
#define DS3231_I2C_ADDR 0x68
#define DS3231_TEMP_MSB 0x11

RTC_DS3231 rtc;

// Which pin on the Arduino is connected to the NeoPixels?
#define NPPIN 8 // On Trinket or Gemma, suggest changing this to 1

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 1 // Popular NeoPixel ring size

Adafruit_NeoPixel NP(NUMPIXELS, NPPIN, NEO_GRB + NEO_KHZ800);

/* Scoots BMP Splash Logo */
const uint8_t logo_bmp[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x06, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x0e, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x0f, 0x00,
  0x00, 0x00, 0x30, 0x00, 0xc0, 0x60, 0x18, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x20, 0x30, 0x1f, 0x00,
  0x00, 0x03, 0xf0, 0x00, 0xe0, 0x70, 0x18, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x30, 0x70, 0x3f, 0x00,
  0x00, 0x0f, 0xf0, 0x10, 0xe0, 0x70, 0x3c, 0x03, 0xfc, 0x00, 0x00, 0x00, 0x30, 0x70, 0x3f, 0x00,
  0x00, 0x1f, 0xe0, 0x30, 0x70, 0x38, 0x3c, 0x07, 0xf8, 0x00, 0x00, 0x00, 0x30, 0x60, 0x3f, 0x00,
  0x00, 0x3f, 0xa0, 0x30, 0xf0, 0x79, 0xff, 0x0f, 0xe8, 0x00, 0x00, 0x00, 0x38, 0xe0, 0x37, 0x00,
  0x00, 0x78, 0x00, 0x71, 0xf8, 0xfd, 0xff, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x38, 0xe0, 0x27, 0x00,
  0x00, 0x78, 0x01, 0xf1, 0xf8, 0xfd, 0xfe, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x38, 0xe0, 0x27, 0x00,
  0x00, 0x78, 0x03, 0xf3, 0xd9, 0xec, 0x7c, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x1c, 0xe0, 0x07, 0x00,
  0x00, 0x7c, 0x03, 0xf3, 0x9d, 0xce, 0x3c, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x1c, 0xc0, 0x07, 0x00,
  0x00, 0x3f, 0xc7, 0x83, 0x9d, 0xce, 0x1c, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x1e, 0xc0, 0x07, 0x00,
  0x00, 0x0f, 0xe7, 0x83, 0x1d, 0x8e, 0x1c, 0x03, 0xf8, 0x00, 0x00, 0x00, 0x1f, 0xc0, 0x07, 0x00,
  0x00, 0x07, 0xe7, 0x07, 0x1f, 0x8e, 0x1c, 0x01, 0xf8, 0x07, 0xff, 0x80, 0x0f, 0xc0, 0x07, 0x00,
  0x00, 0x00, 0xfe, 0x07, 0x0f, 0x86, 0x1c, 0x00, 0x3c, 0x07, 0xff, 0xe0, 0x0f, 0xc0, 0x07, 0x00,
  0x00, 0x00, 0x7e, 0x07, 0x0f, 0x86, 0x1c, 0x00, 0x1e, 0x03, 0xff, 0xe0, 0x0f, 0x80, 0x07, 0x00,
  0x00, 0x00, 0x7f, 0x07, 0x0f, 0x86, 0x0e, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x07, 0x80, 0x07, 0x00,
  0x00, 0x00, 0x7f, 0x07, 0x1f, 0x8e, 0x0e, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x07, 0x80, 0x07, 0x00,
  0x00, 0x00, 0xf3, 0x87, 0x1f, 0x8e, 0x0f, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x03, 0x80, 0x07, 0x00,
  0x00, 0x01, 0xf3, 0xc3, 0x1d, 0x8e, 0x07, 0x80, 0x7c, 0x00, 0x00, 0x00, 0x03, 0x80, 0x07, 0x00,
  0x00, 0x07, 0xe1, 0xf3, 0x99, 0xcc, 0x03, 0xc1, 0xf8, 0x00, 0x00, 0x00, 0x03, 0x80, 0x07, 0x00,
  0x00, 0x0f, 0xc0, 0xfb, 0xf9, 0xfc, 0x03, 0xe3, 0xf0, 0x00, 0x00, 0x00, 0x03, 0x80, 0x0f, 0x00,
  0x00, 0x3f, 0x80, 0x79, 0xf8, 0xfc, 0x01, 0xef, 0xe0, 0x00, 0x00, 0x00, 0x01, 0xc0, 0x0f, 0x00,
  0x00, 0x7e, 0x00, 0x19, 0xf0, 0xf8, 0x00, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00,
  0x00, 0x7c, 0x00, 0x09, 0xf0, 0xf8, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00,
  0x00, 0x30, 0x00, 0x0c, 0x20, 0x10, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00,
  0x00, 0x20, 0x00, 0x04, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00
};

bool drawSplash = false;

int throttle_input, throttle_mapped;
float mapped_dac;

// Global Use data
String logName = "";
String dateNow = "";
String timeNow = "";
String theTemp = "";

File logFile;

void setup() {
  NP.begin();
  NP.clear();
  NP.setBrightness(150);
  
  SabertoothTXPinSerial.begin(9600); // 9600 is the default baud rate for Sabertooth packet serial
  
  Serial.begin(115200); //start serial communication
  //while(!Serial){}
  
  analogReadResolution(12); //set the ADC resolution to 12 bits, default is 10

  initRTC();
  initSD();
  initEPD();
  showSplash(1000);
  NP.setPixelColor(0, NP.Color(0, 95, 0));
  NP.show();
}

void loop(){
  //Serial.println();
  Serial.print("Measured throttle value is : ");
  throttle_input = analogRead(A0);
  throttle_mapped = map(throttle_input, 1065, 3195, 0, 127);
  if (throttle_mapped < 0) {throttle_mapped = 0; }
  
  throttle_mapped = constrain(throttle_mapped, 0, 127);
  
  Serial.print(throttle_input); //Read value at ADC pin A1 and print it
  Serial.print(", Mapped throttle value is : ");
  Serial.println(throttle_mapped);
  ST.motor(throttle_mapped);

}

/*
   initRTC Function
   checks RTC presence and
   intitalizes it.
*/
void initRTC() {
  // Serial.println("Init the RTC");
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    // oledPrint("RTC Module not found", 500);
    while (1);
  } else {
    Serial.println("RTC Initialized");
    //oledPrint("RTC Initialized.", 500);
  }

  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  Serial.print("RTC Time: ");
  Serial.println(theTime());
  Serial.print("RTC Date: ");
  Serial.println(theDate());
  Serial.print("RTC Temp: ");
  Serial.println(getRTCTemp());
  
}

/*
   theTime Function
   checks RTC for time and returns
   time in HH:MM:SS am/pm format
*/
String theTime() {
  DateTime now = rtc.now();

  String nT = "";
  unsigned int hr = now.hour();
  unsigned int mn = now.minute();
  unsigned int sc = now.second();
  String tMn = "";
  String tSc = "";
  String ampm = "";

  if (hr >= 13) {
    hr = hr - 12;
    ampm = " pm";
  } else {
    ampm = " am";
  }

  if (hr == 0) {
    hr = 12;
  }

  if (mn < 10) {
    tMn = "0" + String(mn);
  } else {
    tMn = String(mn);
  }

  if (sc < 10) {
    tSc = "0" + String(sc);
  } else {
    tSc = String(sc);
  }

  nT += hr;
  nT += ':';
  nT += tMn;
  nT += ':';
  nT += tSc;
  nT += ampm;

  return nT;
}

/*
   theDate Function
   checks RTC for date and returns
   date in MM:DD:YY format
*/
String theDate() {
  DateTime now = rtc.now();
  String lD = "";
  unsigned int mn = now.month();
  unsigned int dy = now.day();
  unsigned int yr = now.year();

  yr = yr - 2000;

  lD += mn;
  lD += "-";
  lD += dy;
  lD += "-";
  lD += yr;

  return lD;
}

/*
    Function getTemp
    returns approximate temperature
    from DS3231 RTC Module in F.
*/
String getRTCTemp() {
  short temp_msb;
  String temp;

  Wire.beginTransmission(DS3231_I2C_ADDR);
  Wire.write(DS3231_TEMP_MSB);
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDR, 1);
  temp_msb = Wire.read();

  // for readings in C just use temp_msb
  double tmp = (temp_msb * 1.8) + 32;
  temp = String(tmp) + "F";
  return temp;
}

void initSD(){
  Serial.print("Initializing SD card...");
  if (!SD.begin(SD_CS)) {
    Serial.println("Failed!");
    while(1); //die
  } else {
    Serial.println("Passed!");
    setupLogFile();
  }
  
}

/*
   setupLogFile Function
   creates logfile for the
   current date.
*/
void setupLogFile() {
  String logDate = theDate();
  logName = logDate + ".csv";

  if (!SD.exists("/logs/")) {
    Serial.println("logs folder missing!");
    Serial.println("creating logs folder");
    //oledPrint("logs folder missing!", 500);
    //oledPrint("creating logs folder", 500);
    if (SD.mkdir("/logs/")) {
      //oledPrint("created logs folder", 500);
      Serial.println("created logs folder");
    } else {
      //oledPrint("error making folder", 0);
      Serial.println("error making folder");
      while (1);
    }
  } else {
    Serial.println("logs folder present!");
  }

  logFile = SD.open("/logs/" + logName, FILE_WRITE);
  if (logFile.size() == 0) {
    // newly created, add titles
    /*
    String title_str = "";
    int titles_len = 18;
    for(int i = 0; i < titles_len;i++) {
      title_str += logTitles[i];
      if (i != (titles_len - 1)) {
        title_str += ",";
      }
    }
    logFile.println(title_str);
    */
    logFile.close();
  } else {
    // just close for now.
    logFile.close();
  }

  if (SD.exists("/logs/" + logName)) {
    Serial.println(logName + " present!");
    //oledPrint(logName + " present!", 500);
  } else {
    Serial.println(logName + " missing!");
    //oledPrint(logName + " missing!", 0);
    while (1);
  }

  if (SD.exists("/splash.bmp")) {
    Serial.println("Splash Logo present!");
    //oledPrint(logName + " present!", 500);
    drawSplash = true;
  } else {
    Serial.println("Splash Logo missing!");
    //oledPrint(logName + " missing!", 0);
  }
}

void initEPD(){
  Serial.println("init ePD");
  epd.begin();
  epd.clearBuffer();  
  epd.fillScreen(EPD_WHITE);
  epd.setTextWrap(true);
  epd.setTextSize(1);
}

void showSplash(int dly) {
  if (drawSplash) {
    epd.clearBuffer();  
    epd.fillScreen(EPD_WHITE);
    bmpDraw("/splash.bmp",0,0);
    Serial.println("picture drawn. waiting...");
    delay(dly);
    Serial.println("end waiting.");
  }
}

//this function converts a user entered voltage value into a 10 bit DAC value 
int setDAC(float volt) {
 //formula for calculating DAC output voltage Vdac = (dVal / 1023)*3.3V
 return (int)((volt*1023)/3.3);
}

//This function takes and ADC integer value (0 to 4095) and turns it into a voltage level. The input is the measured 12 bit ADC value.
float convertToVolt(int aVAL) {
 return (((float)aVAL/4095)*3.3); //formula to convert ADC value to voltage reading
}

/* EPD Stuff */

bool bmpDraw(char *filename, int16_t x, int16_t y) {

  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col, x2, y2, bx1, by1;
  uint8_t  r, g, b;
  uint32_t pos = 0, startTime = millis();

  if((x >= epd.width()) || (y >= epd.height())) return false;

  Serial.println();
  Serial.print(F("Loading image '"));
  Serial.print(filename);
  Serial.println('\'');

  // Open requested file on SD card
  if ((bmpFile = SD.open(filename)) == NULL) {
    Serial.print(F("File not found"));
    return false;
  }

  // Parse BMP header
  if(read16(bmpFile) == 0x4D42) { // BMP signature
    Serial.print(F("File size: ")); Serial.println(read32(bmpFile));
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    Serial.print(F("Image Offset: ")); Serial.println(bmpImageoffset, DEC);
    // Read DIB header
    Serial.print(F("Header size: ")); Serial.println(read32(bmpFile));
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if(read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      Serial.print(F("Bit Depth: ")); Serial.println(bmpDepth);
      if((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!
        Serial.print(F("Image size: "));
        Serial.print(bmpWidth);
        Serial.print('x');
        Serial.println(bmpHeight);

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if(bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        x2 = x + bmpWidth  - 1; // Lower-right corner
        y2 = y + bmpHeight - 1;
        if((x2 >= 0) && (y2 >= 0)) { // On screen?
          w = bmpWidth; // Width/height of section to load/display
          h = bmpHeight;
          bx1 = by1 = 0; // UL coordinate in BMP file
  
          for (row=0; row<h; row++) { // For each scanline...
  
            // Seek to start of scan line.  It might seem labor-
            // intensive to be doing this on every line, but this
            // method covers a lot of gritty details like cropping
            // and scanline padding.  Also, the seek only takes
            // place if the file position actually needs to change
            // (avoids a lot of cluster math in SD library).
            if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
              pos = bmpImageoffset + (bmpHeight - 1 - (row + by1)) * rowSize;
            else     // Bitmap is stored top-to-bottom
              pos = bmpImageoffset + (row + by1) * rowSize;
            pos += bx1 * 3; // Factor in starting column (bx1)
            if(bmpFile.position() != pos) { // Need seek?
              bmpFile.seek(pos);
              buffidx = sizeof(sdbuffer); // Force buffer reload
            }
            for (col=0; col<w; col++) { // For each pixel...
              // Time to read more pixel data?
              if (buffidx >= sizeof(sdbuffer)) { // Indeed
                bmpFile.read(sdbuffer, sizeof(sdbuffer));
                buffidx = 0; // Set index to beginning
              }
              // Convert pixel from BMP to EPD format, push to display
              b = sdbuffer[buffidx++];
              g = sdbuffer[buffidx++];
              r = sdbuffer[buffidx++];

              uint8_t c = 0;
              if ((r < 0x80) && (g < 0x80) && (b < 0x80)) {
                 c = EPD_BLACK; // try to infer black
              } else if ((r >= 0x80) && (g >= 0x80) && (b >= 0x80)) {
                 c = EPD_WHITE;
              } else if (r >= 0x80) {
                c = EPD_RED; //try to infer red color
              }
              
              epd.writePixel(col, row, c);
            } // end pixel
          } // end scanline
        } // end onscreen
        epd.display();
        Serial.print(F("Loaded in "));
        Serial.print(millis() - startTime);
        Serial.println(" ms");
      } // end goodBmp
    }
  }

  bmpFile.close();
  if(!goodBmp) {
    Serial.println(F("BMP format not recognized."));
    return false;
  }
  return true;
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}


void printDirectory(File dir, int numTabs) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
} 
