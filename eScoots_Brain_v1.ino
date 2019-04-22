//***************ZeroDACExample Sketch*******************************
//This sketch provides an example on using the DAC on the Arduino Zero.
//It was used in a video tutorial on the ForceTronics YouTube Channel.
//This code is free and open for anybody to use and modify at their own risk

#if ARDUINO_ARCH_ESP32
#include <Arduino.h>
#include <analogWrite.h>
#endif

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(128, 32, &Wire, -1);

static const unsigned char PROGMEM logo_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000 };

int throttle_input, throttle_mapped;
float mapped_dac;

void setup() {
  Serial.begin(115200); //start serial communication
  //while(!Serial){}

  //showSplash(0);
  
  analogReadResolution(12); //set the ADC resolution to 12 bits, default is 10
  
  #if ARDUINO_ARCH_ESP32
  analogWriteResolution(10,A0); //set the Arduino DAC for 10 bits of resolution (max)
  #else
  analogWriteResolution(10); //set the Arduino DAC for 10 bits of resolution (max)
  #endif
  
  //Get user entered voltage, convert to DAC value, output DAC value
  analogWrite(A0,setDAC(0));
  delay(1000);
  
  //display.clearDisplay();
  //display.display();
}

void loop(){
  //Serial.println();
  Serial.print("Measured throttle value is : ");
  throttle_input = analogRead(A1);
  #if ARDUINO_ARCH_ESP32
  throttle_mapped = map(throttle_input, 880, 2990, 0, 4095);
  #else
  throttle_mapped = map(throttle_input, 1085, 3210, 0, 4095);
  #endif
  if (throttle_mapped < 0) {throttle_mapped = 0; }
  
  mapped_dac = convertToVolt(throttle_mapped);
  
//  if (mapped_dac < 0.00) { mapped_dac = 0.00; }
//  if (mapped_dac > 3.305) { mapped_dac = 3.303; }

  mapped_dac = constrain(mapped_dac, 0.00, 3.303);
  
  //Serial.println(mapped_dac); //Read value at ADC pin A1 and print it
  analogWrite(A0,setDAC(mapped_dac));
  Serial.print(throttle_input); //Read value at ADC pin A1 and print it
  Serial.print(", Mapped throttle value is : ");
  Serial.print(throttle_mapped);
  Serial.print(", Mapped throttle voltage is : ");
  Serial.println(setDAC(mapped_dac));

}

void showSplash(int timeDelay){
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    //for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(timeDelay); // Pause
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
