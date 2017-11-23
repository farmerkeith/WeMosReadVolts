// File created 8 Nov 2017
// This object is designed to be called on every loop

class WeMosVolts {
  public:
  WeMosVolts(); // constructor
  // functions
  void run();
  void calibrateZero();
  void calibrateScale(unsigned long mV);
  float getVolts(); // returns Volts
  unsigned long getMilliVolts(long &temp1); // returns milli volts; parameter returns ADC code*oversampling
  void returnToNormal();
  // variables
  unsigned long fullScale = 6158;  // mV, max 2^16-1
  long zeroOffset=500; // ADC output code
  float zeroOffsetMin=1000; // ADC output code * oversampling
  float zeroOffsetMax=0; // ADC output code * oversampling
  float zeroOffsetExp=zeroOffset; // ADC output code * oversampling
  long milliVolts = 0;
  int taskPeriod = 100;
  unsigned long voltsTime=0;
  const int filter = 16; // exponential decay filter parameter; 1 for no filtering
  const byte filterLevel = 4; // No. of filter stages in series; max is 4
  private:
  void filterAdc();
  void printAdcLine();
  const int ignoreThreshold = 500; // adc Code * 100
  unsigned long counter = 0, oldCounter = 0;
  unsigned long adcCode = 0;
  unsigned long adcCodeArray[11];
  unsigned long filteredAdcCode = 0;
  unsigned long filteredAdcCodeC = 0;
  unsigned long filteredAdcCodeB = 0;
  unsigned long filteredAdcCodeA = 0;
  enum voltState {
    voltState_normal,
    voltState_zero,
    voltState_scale
//    voltState_waitZero,
//    voltState_waitScale
  } voltState ;
  byte lastState = voltState_normal;
  
};

WeMosVolts WeMosVolts; // instantiates class as object WeMosVolts

// constructor
WeMosVolts::WeMosVolts(){
  filteredAdcCode = analogRead(A0)*100; // get initial value close to final
  filteredAdcCodeA = filteredAdcCode ;
  filteredAdcCodeB = filteredAdcCode ;
  filteredAdcCodeC = filteredAdcCode ;
  for (int i=0; i<11; i++){
     adcCodeArray[i]=0;
  }
  while (millis()>voltsTime){ // increment start time to just after now
    voltsTime += 2000;
  }
}
// functions
void WeMosVolts::run(){
  if (lastState != voltState){
    delay(20); // ToDo - get rid of delay(), replace with a state and timer
    filteredAdcCode = analogRead(A0)*100;
    filteredAdcCodeA = filteredAdcCode;
    filteredAdcCodeB = filteredAdcCode;
    filteredAdcCodeC = filteredAdcCode;
    lastState = voltState;
    Serial.print (" filteredAdcCode reset to ");
    Serial.println ((float)filteredAdcCode/100);
  }
  if((long)(millis() - voltsTime) >= 0){ // time is expired
    voltsTime += taskPeriod; // ms, set time for the following execution
    adcCode = analogRead(A0)*100; // multiply by 100 to increase calculation accuracy
    counter ++;
    for (int i=10; i>0; i--){
       adcCodeArray[i]=adcCodeArray[i-1];
    }
    adcCodeArray[0]=adcCode;

/*
    if (((adcCode-filteredAdcCode)>ignoreThreshold)&&((filteredAdcCode-adcCode)>ignoreThreshold)) {      
      Serial.print("ignoreThreshold exceeded");
      Serial.print(" counter ");
      Serial.print(counter);
      Serial.print(" adcCode-filteredAdcCode ");
      Serial.print((long)(adcCode-filteredAdcCode));
      Serial.print(" ignoreThreshold ");
      Serial.print(ignoreThreshold);
      Serial.print(" adcCode ");
      Serial.print(adcCode);
      Serial.print(" filteredAdcCode ");
      Serial.println(filteredAdcCode);
      
    }
*/    
    // filtering if reading is close to filtered value
    if (((adcCode-filteredAdcCode)<ignoreThreshold)||((filteredAdcCode-adcCode)<ignoreThreshold)) {
      filterAdc();
      
    } else { // first reading after a good one will be ignored
      if ((counter-oldCounter)>1){
        oldCounter = counter ;
//        Serial.print(" discarding ");
//        Serial.print(adcCode);
//        Serial.print(" average ");
//        Serial.println(filteredAdcCode);
      } else {
        filterAdc();
//        Serial.print(" accepting ");
//        Serial.print(adcCode);
//        Serial.print(" average ");
//        Serial.println(filteredAdcCode);
      }
    }
    milliVolts = (((long)(filteredAdcCode -zeroOffset)*(long)fullScale) / (1024-zeroOffset/100));
//    printAdcLine();
  }
}

void WeMosVolts::printAdcLine(){
  const int adcBase = 74000;
  char S1='*';
  Serial.print ("T= ");
  Serial.print (millis());
  Serial.print (" mV= ");
  Serial.print ((float)milliVolts/100);
  Serial.print (" adc= ");
  Serial.print (adcCode);
  Serial.print (" ");
  int a1 = ((long)adcCode-adcBase)/100;
  if (a1<0) a1 = 0; 
  for (int i=0; i<a1; i++){
    Serial.print ("*");
  }
  for (int i=0; i<(20-a1); i++){
    Serial.print (" ");
  }
//  Serial.print ((long)adcCode-adcBase);
  Serial.print (" filtered ");
  Serial.print (filteredAdcCode);
  Serial.print (" ");
  
  a1 = (((long)filteredAdcCode-adcBase)/5)%50;
  if (a1<0) a1 = 0; 
  if (filteredAdcCode-adcBase<750) S1='-';
  if (filteredAdcCode-adcBase>1000) S1='0';
  for (int i=0; i<a1; i++){
    Serial.print (S1);
  }
//  Serial.print ((long)filteredAdcCode-adcBase);
  Serial.println ();
}

// _____________________________
void WeMosVolts::filterAdc(){
//  filteredAdcCode = adcCode; // unfiltered (for testing)
  
  filteredAdcCodeA = (filteredAdcCodeA * (filter-1) + adcCode)/filter;
  filteredAdcCodeB = (filteredAdcCodeB * (filter-1) + filteredAdcCodeA)/filter;
  filteredAdcCodeC = (filteredAdcCodeC * (filter-1) + filteredAdcCodeB)/filter;
  filteredAdcCode = (filteredAdcCode * (filter-1) + filteredAdcCodeC)/filter;

}

// _____________________________
void WeMosVolts::calibrateZero(){ 
  Serial.print ("calibrate Zero reading ");
  voltState = voltState_zero;
  delay(500); // time for calibration circuit to settle
  unsigned long zeroCode = analogRead(A0)*100; // discard first reading
//  zeroCode = 0;
  zeroOffsetExp = zeroCode; // initialise Exp
  Serial.print (" ");
  Serial.print (zeroCode);
  Serial.print ("; ");
  
  for (int i=0; i<16; i++){
    zeroCode = analogRead(A0)*100;
    zeroOffsetExp = zeroOffsetExp*7/8 + zeroCode/8; // set Exp 
    delay(20);
    Serial.print (" ");
    Serial.print (zeroCode);
  }
  Serial.println();
//  if (zeroCode <zeroOffsetMin) zeroOffsetMin=temp; // set Min 
//  if (zeroCode>zeroOffsetMax) zeroOffsetMax=temp; // set Min 
//  zeroOffsetExp = zeroOffsetExp*7/8 + zeroCode/8; // set Exp 
  if (zeroOffsetExp<1000) zeroOffset = zeroOffsetExp; // result
  filteredAdcCode = zeroOffset;  
  Serial.print ("zeroOffset set to ");
  Serial.println (zeroOffset);
}

void WeMosVolts::calibrateScale(unsigned long mV){
    
}
  
float WeMosVolts::getVolts(){ // returns Volts
  long _temp=0;
  return (float)getMilliVolts(_temp)/1000;
}

unsigned long WeMosVolts::getMilliVolts(long &temp){ 
   // returns milli volts; parameter returns ADC code*oversampling
/*   
   Serial.print(" object filteredAdcCode ");
   Serial.print(filteredAdcCode);
   Serial.print(" C ");
   Serial.print(filteredAdcCodeC);
   Serial.print(" B ");
   Serial.print(filteredAdcCodeB);
   Serial.print(" A ");
   Serial.print(filteredAdcCodeA);
   Serial.print(" last ");
   Serial.print(adcCode);
    for (int i=0; i<11; i++){
       Serial.print (" ");
       Serial.print (adcCodeArray[i]);
    }
   Serial.println();
*/   
   temp = filteredAdcCode;
   return milliVolts;
}

void WeMosVolts::returnToNormal(){
  voltState = voltState_normal;
}


