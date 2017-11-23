// File created 8 Nov 2017
// This object is designed to be called on every loop
// Major update 23 Nov 2017 for 2 modes of operation.
// Mode 1: regular (frequent) measurements with exponential decay filtering and 
// discard of measurements that are too far away from the current mean. 
// readVoltage() delivers the latest filtered value. 
// Mode 2: no regular measurements. On command the ADC measures a number of times, 
// discarding measurements too far away from the mean. 


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
  unsigned long fullScale = 6156;  // mV, max 2^16-1
  
  int taskPeriod = 20; // interval between measurements (default: 20 ms)
  int filter = 16; // exponential decay filter parameter; 1 for no filtering (default 16, can be 1 up to 1024) (?) 
  byte filterLevel = 4; // No. of filter stages in series; max is 4 (default 4, can be 1 up to 8).
  int ignoreThreshold = 500; // adc Code * 100 :discard criterion (0 for no discard; 1,2,3 etc for allowable distance from mean; 
  byte discardLimit=3; //  limit on multiple successive discards (default: 3)

  long zeroOffset=500; // ADC output code
  float zeroOffsetMin=1000; // ADC output code * oversampling
  float zeroOffsetMax=0; // ADC output code * oversampling
  float zeroOffsetExp=zeroOffset; // ADC output code * oversampling
  long milliVolts = 0;
  unsigned long voltsTime=0;
  private:
  void filterAdc(); // ToDo: make variable filter stages
  void printAdcLine();
  unsigned long counter = 0, oldCounter = 0;
  unsigned long adcCode = 0;
  unsigned long adcCodeArray[11]; // history of adc codes
  unsigned long filteredAdcCode = 0; // ToDo: turn into array for variable filter stages
  unsigned long filteredAdcCodeC = 0;
  unsigned long filteredAdcCodeB = 0;
  unsigned long filteredAdcCodeA = 0;
  enum voltState { // state variable to manage zero and scale calibration
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
      // ToDo: make a parameter for discard limit
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
void WeMosVolts::filterAdc(){ // ToDo: make variable from 1 to 8
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
   // ToDo: change parameter to control Mode 1 (==0) or Mode 2 (>0)
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


