//***************ZeroDACExample Sketch*******************************
//This sketch provides an example on using the DAC on the Arduino Zero.
//It was used in a video tutorial on the ForceTronics YouTube Channel.
//This code is free and open for anybody to use and modify at their own risk

int throttle_input, throttle_mapped;
float mapped_dac;

void setup()
{
  Serial.begin(9600); //start serial communication
  //while(!Serial){}
  analogWriteResolution(10); //set the Arduino DAC for 10 bits of resolution (max)
  analogReadResolution(12); //set the ADC resolution to 12 bits, default is 10
  
  //Get user entered voltage, convert to DAC value, output DAC value
  analogWrite(A0,setDAC(0));
  delay(2500);
}

void loop(){
  //Serial.println();
  //Serial.print("Measured throttle value is : ");
  throttle_input = analogRead(A1);
  throttle_mapped = map(throttle_input, 1083, 3195, 0, 4095);
  mapped_dac = convertToVolt(throttle_mapped);
  
  if (mapped_dac < 0.00) { mapped_dac = 0.00; }
  if (mapped_dac > 3.20) { mapped_dac = 3.20; }
  
  //Serial.println(mapped_dac); //Read value at ADC pin A1 and print it
  analogWrite(A0,setDAC(mapped_dac));
//  Serial.print(throttle_input); //Read value at ADC pin A1 and print it
//  Serial.print(", Mapped throttle value is : ");
//  Serial.print(throttle_mapped);
//  Serial.print(", Mapped throttle voltage is : ");
//  Serial.println(setDAC(throttle_mapped));

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
