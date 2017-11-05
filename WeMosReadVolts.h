// file created 26 October 2017 by farmerkeith
// last update 5 Nov 2017

class WeMosVolts {
  public:
  WeMosVolts();
  void calibrateZero();
  void calibrateScale(unsigned long mV);
  float getVolts(); // returns Volts
  unsigned long getMilliVolts(); // returns milli volts
  unsigned long fullScale = 6244;  // mV, max 2^16-1
//  float zeroOffset=5; // ADC output code
  unsigned long zeroOffset=320; // ADC output code * 64
  float zeroOffsetMin=1000; // ADC output code * 64
  float zeroOffsetMax=0; // ADC output code * 64
  float zeroOffsetExp=320; // ADC output code * 64
//  private:
};

// constructor
WeMosVolts::WeMosVolts(){
}

// functions
void WeMosVolts::calibrateZero(){ 
  // assumes input is connected to 0 volts
//  float temp = 0;
  int temp = 0;
  temp = analogRead(A0); // discard first reading
  temp = 0;
  Serial.print (" calibrate Zero readings ");
  for (int i=0; i<64; i++){
    int temp1 = analogRead(A0);
    temp += temp1;
    delay(1);
//    Serial.print (" ");
//    Serial.print (temp1);
  }
//  Serial.println();
  if (temp<zeroOffsetMin) zeroOffsetMin=temp; // set Min 
  if (temp>zeroOffsetMax) zeroOffsetMax=temp; // set Min 
  zeroOffsetExp = zeroOffsetExp*7/8 + temp/8; // set Exp 

//  (temp +=1) /= 64;  
  zeroOffset = temp; // 64 * average reading over 64 measurements
}

void WeMosVolts::calibrateScale(unsigned long mV){
  // assumes input is connected to mV milli volts
  
  unsigned long temp = analogRead(A0);
  temp = 0;
  delay(1);
  Serial.print (" calibrate Scale readings ");
  for (int i=0; i<64; i++){
    int temp1 = analogRead(A0);
    temp += temp1;
    delay(1);
//    Serial.print (" ");
//    Serial.print (temp1);
  }
//  Serial.println(); // temp = 64*adc for specified voltage (mV)
//  temp /=10;
  fullScale = (mV * (64*1024-zeroOffset))/(temp-zeroOffset);  

//   Serial.print (" fullScale=");
//  Serial.print (fullScale);
//  Serial.print (" ");
//  Serial.println ((float)(mV * (1024-zeroOffset))/(1000-zeroOffset));
}

unsigned long WeMosVolts::getMilliVolts(){ // returns milli volts
  // Serial.print (" return=");
  // Serial.print (((1000-zeroOffset)*fullScale) / (1024-zeroOffset)+1);
  // Serial.print (" ");
  // Serial.println (((float)(1000-zeroOffset)*fullScale) / (1024-zeroOffset)/100,4);
// return (((1000          -zeroOffset)*fullScale) / (1024-zeroOffset)+1)/100;  
    unsigned long temp = analogRead(A0); // discard first reading
    temp = 0;
    for (int i=0; i<64; i++){
      temp += analogRead(A0);
    }
//    Serial.print (" line 81 analogRead=");
//    Serial.println (temp);
    return (((temp -zeroOffset)*fullScale) / (64*1024-zeroOffset));  
//    return (((analogRead(A0)-zeroOffset)*fullScale) / (1024-zeroOffset)+1)/100;  
  
}

float WeMosVolts::getVolts(){ // returns Volts
  return (float)getMilliVolts()/1000;
}

