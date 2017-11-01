// file created 26 October 2017 by farmerkeith
class WeMosVolts {
  public:
  WeMosVolts();
  void calibrateZero();
  void calibrateScale(long mV);
  float getVolts(); // returns Volts
  float getMilliVolts(); // returns milli volts
  long fullScale = 617371;  // mV
  int zeroOffset=9; // ADC output code
//  private:
};

// constructor
WeMosVolts::WeMosVolts(){
}

// functions
void WeMosVolts::calibrateZero(){ 
  // assumes input is connected to 0 volts
//  zeroOffset = 25;
  int temp = analogRead(A0);
  Serial.print (" line 23 analogRead=");
  Serial.println (temp);
  
  zeroOffset = temp;
//  zeroOffset = analogRead(A0);
}

void WeMosVolts::calibrateScale(long mV){
  // assumes input is connected to mV milli volts
  //    fullScale = (100*mV * (1024-zeroOffset))/(1000          -zeroOffset);  
  int temp = analogRead(A0);
  Serial.print (" line 34 analogRead=");
  Serial.println (temp);
  fullScale = (100*mV * (1024-zeroOffset))/(temp-zeroOffset);  
//  fullScale = (100*mV * (1024-zeroOffset))/(analogRead(A0)-zeroOffset);  

//   Serial.print (" fullScale=");
//  Serial.print (fullScale);
//  Serial.print (" ");
//  Serial.println ((float)(mV * (1024-zeroOffset))/(1000-zeroOffset));
}

float WeMosVolts::getMilliVolts(){ // returns milli volts
  // Serial.print (" return=");
  // Serial.print (((1000-zeroOffset)*fullScale) / (1024-zeroOffset)+1);
  // Serial.print (" ");
  // Serial.println (((float)(1000-zeroOffset)*fullScale) / (1024-zeroOffset)/100,4);
// return (((1000          -zeroOffset)*fullScale) / (1024-zeroOffset)+1)/100;  
    int temp = analogRead(A0);
    Serial.print (" line 52 analogRead=");
    Serial.println (temp);
    return (((temp -zeroOffset)*fullScale) / (1024-zeroOffset)+1)/100;  
//    return (((analogRead(A0)-zeroOffset)*fullScale) / (1024-zeroOffset)+1)/100;  
  
}

float WeMosVolts::getVolts(){ // returns Volts
  return (float)getMilliVolts()/1000;
}

