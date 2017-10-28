// file created 26 October 2017 by farmerkeith
class WeMosVolts {
  public:
  WeMosVolts();
  void calibrateZero();
  void calibrateScale(int mV);
  float getVolts(); // returns Volts
  int getMilliVolts(); // returns milli volts
  int fullScale = 3200;  // mV
  private:
  int zeroOffset=0; // ADC output code
};

// constructor
WeMosVolts::WeMosVolts(){
}

// functions
void WeMosVolts::calibrateZero(){ 
  // assumes input is connected to 0 volts
  zeroOffset = analogRead(A0);
}

void WeMosVolts::calibrateScale(int mV){
  // assumes input is connected to mV milli volts
  fullScale = analogRead(A0);  
}

int WeMosVolts::getMilliVolts(){ // returns milli volts
  return (analogRead(A0)-zeroOffset)*fullScale / (1023-zeroOffset);  
}

float WeMosVolts::getVolts(){ // returns Volts
  return (float)getMilliVolts()/1000;
}

