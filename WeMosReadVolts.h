// file created 26 October 2017 by farmerkeith
// last update 5 Nov 2017

class WeMosVolts {
  public:
  WeMosVolts(); // constructor
  void calibrateZero();
  void calibrateScale(unsigned long mV);
  float getVolts(); // returns Volts
  unsigned long getMilliVolts(long &temp1); // returns milli volts
  unsigned long fullScale = 6158;  // mV, max 2^16-1
//  float zeroOffset=5; // ADC output code
  long zeroOffset=320; // ADC output code * 64
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
  Serial.print("Scale analogRead(A0) total code ");
  Serial.print(temp);
  Serial.print(" average ");
  Serial.println((float)temp/64);
  fullScale = (mV * (64*1024-zeroOffset))/(temp-zeroOffset);  

//   Serial.print (" fullScale=");
//  Serial.print (fullScale);
//  Serial.print (" ");
//  Serial.println ((float)(mV * (1024-zeroOffset))/(1000-zeroOffset));
}

unsigned long WeMosVolts::getMilliVolts(long &temp){ // returns milli volts
    temp = analogRead(A0); // discard first reading
    temp = 0;
    for (int i=0; i<64; i++){
      temp += analogRead(A0);
    }
//    Serial.print("getMilliVolts analogRead(A0) average ");
//    Serial.println((float)temp/64);
    if (temp-zeroOffset<=0) return 0; 
    else 
    return (((temp -zeroOffset)*(long)fullScale) / (64*1024-zeroOffset));  
}

float WeMosVolts::getVolts(){ // returns Volts
  long _temp=0;
  return (float)getMilliVolts(_temp)/1000;
}

