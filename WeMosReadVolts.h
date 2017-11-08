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
  long zeroOffset=5 * oversampling; // ADC output code * oversampling
  float zeroOffsetMin=1000; // ADC output code * oversampling
  float zeroOffsetMax=0; // ADC output code * oversampling
  float zeroOffsetExp=zeroOffset; // ADC output code * oversampling
  byte oversampling = 64; // No. of measurements per report
//  private:
};

// constructor
WeMosVolts::WeMosVolts(){
}

// functions
void WeMosVolts::calibrateZero(){ 
  // assumes input is connected to 0 volts
  Serial.print (" calibrate Zero readings ");
  delay(200); // time for calibration circuit to settle
  unsigned long temp = analogRead(A0); // discard first reading
  temp = 0;
  for (int i=0; i<oversampling; i++){
    int temp1 = analogRead(A0);
    temp += temp1;
//    delay(1);
    yield();
    Serial.print (" ");
    Serial.print (temp1);
  }
  Serial.println();
  if (temp<zeroOffsetMin) zeroOffsetMin=temp; // set Min 
  if (temp>zeroOffsetMax) zeroOffsetMax=temp; // set Min 
  zeroOffsetExp = zeroOffsetExp*7/8 + temp/8; // set Exp 

//  (temp +=1) /= 64;  
  zeroOffset = temp; // oversampling * average reading
}

void WeMosVolts::calibrateScale(unsigned long mV){
  // assumes input is connected to mV milli volts
  Serial.print (" calibrate Scale readings ");
  delay(300); // time for calibration circuit to settle
  int val[oversampling];
  unsigned long temp = analogRead(A0); // discard first reading
  temp = 0;
  for (int i=0; i<oversampling; i++){
    int temp1 = analogRead(A0);
    temp += temp1;
    yield();
    val[i] = temp1;
  }
  for (int i=0; i<oversampling; i++){
    Serial.print (" ");
    Serial.print (val[i]);
  }
  Serial.println ();
  Serial.print("Scale analogRead(A0) total code ");
  Serial.print(temp);
  Serial.print(" average ");
  Serial.println((float)temp/oversampling);
  fullScale = (mV * (oversampling*1024-zeroOffset))/(temp-zeroOffset);  

}

unsigned long WeMosVolts::getMilliVolts(long &temp){ // returns milli volts
  int val[oversampling];
  temp = analogRead(A0); // discard first reading
  temp = 0;
  for (int i=0; i<oversampling; i++){
    val[i]= analogRead(A0);
    temp += val[i];
    yield();
    delay(20);
  }
//  for (int i=0; i<oversampling; i++){
//    Serial.print (" ");
//    Serial.print (val[i]);
//  }
//  Serial.println ();
    if (temp-zeroOffset<=0) return 0; 
    else 
    return (((temp -zeroOffset)*(long)fullScale) / (oversampling*1024-zeroOffset));  
}

float WeMosVolts::getVolts(){ // returns Volts
  long _temp=0;
  return (float)getMilliVolts(_temp)/1000;
}

