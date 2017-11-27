// File created 8 Nov 2017
// This object is designed to be called on every loop
// Major update 23 Nov 2017 for 2 modes of operation.
// Mode 1: regular (frequent) measurements with exponential decay filtering and 
// discard of measurements that are too far away from the current mean. 
// readVoltage(0) delivers the latest filtered value. 
// Mode 2: no regular measurements. On command the ADC measures a number of times, 
// discarding measurements too far away from the mean. 
// in Mode 2, readVoltage(1) does a single measurement with no filtering
// in Mode 2, readVoltage(2) or readVoltage(n) does a series of measurements lasting n milliseconds
// discarding readings that are more than ignoreThreshold away from the mean


class WeMosVolts {
  public:
  WeMosVolts(); // constructor
  // functions
  void run();
  void calibrateZero();
  void calibrateScale(unsigned long mV);
  float getVolts(byte timeAllowed); // returns Volts
  unsigned long getMilliVolts(byte timeAllowed); // returns milli volts; parameter returns ADC code*oversampling
  unsigned long getVoltCode(byte timeAllowed); // returns voltCode
  void returnToNormal();
  
  // variables
  unsigned long fullScale = 6156;  // mV, max 2^16-1
  
  int taskPeriod = 20; // interval between measurements (default: 20 ms)
  byte filter = 4; // exponential decay filter parameter; 1 for no filtering (default 16, can be 1 up to 1024) (?) 
  byte filterLevel = 4; // No. of filter stages in series; max is 4 (ToDo: default 4, can be 1 up to 8).
  int ignoreThreshold = 5; // adc Code :discard criterion (0 for no discard; 1,2,3 etc for allowable distance from mean; 
  byte discardLimit=3; //  limit on multiple successive discards (default: 3)
  byte timeAllowed=1; // Mode 2

  long zeroOffset=500; // ADC output code
  float zeroOffsetMin=1000; // ADC output code * oversampling
  float zeroOffsetMax=0; // ADC output code * oversampling
  float zeroOffsetExp=zeroOffset; // ADC output code * oversampling
  long milliVolts = 0;
  unsigned long voltsTime=0;
  unsigned long filteredAdcCodeArray[8];
  
  private:
  unsigned long filterAdc(unsigned long _adcCode, byte filter);
  void printAdcLine();
  unsigned long counter = 0, oldCounter = 0;
  unsigned long adcCode = 0;
  unsigned long adcCodeArray[11]; // history of adc codes
  unsigned long filteredAdcCode = 0; // ToDo: turn into array for variable filter stages
  bool mode1=0; // set to 0 by run(); zero means mode 2.
  enum voltState { // variable type to manage zero and scale calibration state machine
    voltState_normal,
    voltState_zero,
    voltState_scale, 
//    voltState_settling
//    voltState_waitZero,
//    voltState_waitScale
  }; //
  voltState currentState = voltState_normal; 
  voltState lastState = voltState_zero;
  bool voltsSettling = 0;
  
};

WeMosVolts WeMosVolts; // instantiates class as object WeMosVolts

// constructor
WeMosVolts::WeMosVolts(){
  adcCode = analogRead(A0)*100; // multiply by 100 to increase calculation accuracy
  filteredAdcCode = filterAdc(adcCode, 0); // initialise filter variables
  for (int i=0; i<11; i++){
     adcCodeArray[i]=0;
  }
  while (millis()>voltsTime){ // increment start time to just after now
    voltsTime += 2000;
  }
}
// functions
void WeMosVolts::run(){
  mode1 = 1;
  if (lastState != currentState){
    voltsSettling = 1;
    voltsTime = millis()+20; // set for 20 ms from now 
    lastState = currentState;
  }
  if (voltsSettling==1){ // waiting 20 ms
    if((long)(millis() - voltsTime) >= 0){ // time is expired
      // delay is to allow the hardware to settle at the new voltage
      Serial.print (" filteredAdcCode from contructor was ");
      Serial.println (filteredAdcCode);
    
      adcCode = analogRead(A0)*100; // multiply by 100 to increase calculation accuracy
      filteredAdcCode = filterAdc(adcCode, 0); // initialise filter variables
//      for (int i=0; i<filterLevel; i++) filteredAdcCodeArray[i]= adcCode; // initialise direct memory array
      lastState = currentState;
      voltsSettling = 0;
      Serial.print (" updated AdcCode was ");
      Serial.print (adcCode);
      Serial.print (" filteredAdcCode reset to ");
      Serial.println (filteredAdcCode);
    }
  } else {
    if((long)(millis() - voltsTime) >= 0){ // time is expired
      voltsTime += taskPeriod; // ms, set time for the following execution
      counter ++;
      for (int i=10; i>0; i--){
         adcCodeArray[i]=adcCodeArray[i-1];
      }
      adcCode = analogRead(A0)*100; // multiply by 100 to increase calculation accuracy
      adcCodeArray[0]=adcCode;

/*
    if (((adcCode-filteredAdcCode)>ignoreThreshold*100)&&((filteredAdcCode-adcCode)>ignoreThreshold*100)) {      
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
      if ((ignoreThreshold==0)||((filteredAdcCode+ignoreThreshold*100>=adcCode) && 
          (filteredAdcCode-ignoreThreshold*100<=adcCode))) { // do not discard
        filteredAdcCode = filterAdc(adcCode, filter);
        oldCounter = counter;
//      Serial.print (" line 129 filtered AdcCode returned ");
//      Serial.println (filteredAdcCode);
      } else { // candidate for discard 
            // readings after a good one will be ignored up to discardLimit
        if ((counter-oldCounter)<=discardLimit){ // under limit, discard
//          Serial.print(" discarding ");
//          Serial.print(adcCode);
//          Serial.print(" average ");
//          Serial.println(filteredAdcCode);
        } else { // over limit, do not discard
          filteredAdcCode = filterAdc(adcCode, filter);
          oldCounter = counter ;
//          Serial.print(" accepting ");
//        Serial.print(adcCode);
//        Serial.print(" average ");
//        Serial.println(filteredAdcCode);
        } // end of if ((counter-oldCounter)>discardLimit)
      }
//    milliVolts = (((long)(filteredAdcCode -zeroOffset)*(long)fullScale) / (1024-zeroOffset/100));
//    printAdcLine();
    }
  }
}

// _____________________________
unsigned long WeMosVolts::getVoltCode(byte timeAllowed){ 
   // returns voltCode, averaged vallue of analogRead(A0) * 100
   // ToDo: change parameter to control Mode 1 (==0) or Mode 2 (>0)
  if (timeAllowed==0) return filteredAdcCode;
  else return 5000;
   
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
}

// _____________________________
unsigned long WeMosVolts::getMilliVolts(byte timeAllowed){ 
   // returns milli volts; parameter returns ADC code*oversampling
   // ToDo: change parameter to control Mode 1 (==0) or Mode 2 (>0)
  if (timeAllowed==0&&mode1==1) return (((long)(filteredAdcCode -zeroOffset)*(long)fullScale) / (1024-zeroOffset/100)); // millivolts
  else if (timeAllowed==1 || timeAllowed==0&&mode1==0) {
    return (((long)(analogRead(A0)*100 -zeroOffset)*(long)fullScale) / (1024-zeroOffset/100)); // single reading
  } else { // timeAllowed >=2
    unsigned long startMillis=millis();
    unsigned long inCounter=0, discardCounter=0, outCounter=0;
    unsigned long debugCounter1=0, debugCounter2=0;
    unsigned long adcOutAverage=0, adcOutTotal=0;
    unsigned long adcInAverage=0, adcInTotal=0; 
    unsigned long adcReading[10];
    byte j=0; // index for adcReading
    byte phaseCounter=0;
    for (int i=0; i<10; i++) adcReading[i]=0; // reset array
    while (millis()-startMillis<timeAllowed){
      inCounter++;
      if(phaseCounter==1){ // phase 1 circulates the array
        if (j==10) j=0;
        if((adcInAverage+ignoreThreshold*100>=adcReading[j]) && 
           (adcInAverage-ignoreThreshold*100<=adcReading[j])){
          outCounter ++;  
//          debugCounter1++;
          adcOutTotal+= adcReading[j];
          adcOutAverage = adcOutTotal/outCounter;
/*        
        Serial.print ("WeReadVolts outCount= ");
        Serial.print (outCounter);
        Serial.print (" reading= ");
        Serial.print (adcReading[j]);
        Serial.print (" j= ");
        Serial.print (j);
        Serial.print (" outTotal= ");
        Serial.print (adcOutTotal);
        Serial.println ();
*/
        } else {
          discardCounter++;
        }
        adcReading[j] = analogRead(A0)*100;
        adcInTotal += adcReading[j]; 
        adcInAverage = adcInTotal / inCounter;
        j++;
/*
        Serial.print ("ReadVolts inCount= ");
        Serial.print (inCounter);
        Serial.print (" reading= ");
        Serial.print (adcReading[j-1]);
        Serial.print (" j= ");
        Serial.print (j-1);
        Serial.print (" inTotal= ");
        Serial.print (adcInTotal);
        Serial.println ();
*/
      }  
      if(phaseCounter==0){ // phase 0 puts readings into the array
        adcReading[j] = analogRead(A0)*100;
        adcInTotal += adcReading[j]; 
        adcInAverage = adcInTotal / inCounter;
        j++;
        if (j==10) phaseCounter =1; // array is full
/*
        Serial.print ("MosReadVolts inCount= ");
        Serial.print (inCounter);
        Serial.print (" reading= ");
        Serial.print (adcReading[j-1]);
        Serial.print (" j= ");
        Serial.print (j-1);
        Serial.print (" inTotal= ");
        Serial.print (adcInTotal);
        Serial.println ();
*/
      }
      yield();
    }
    // time is expired
    if(phaseCounter==1){ // the array is full
      for (int i = 0; i<10; i++){
        if (j==10) j=0;
        if((adcInAverage+ignoreThreshold*100>=adcReading[j]) && 
           (adcInAverage-ignoreThreshold*100<=adcReading[j])){
          outCounter ++;
          adcOutTotal+= adcReading[j];
          adcOutAverage = adcOutTotal/outCounter;
          j++;
/*
        Serial.print ("WeReadVolts outCount= ");
        Serial.print (outCounter);
        Serial.print (" reading= ");
        Serial.print (adcReading[j-1]);
        Serial.print (" j= ");
        Serial.print (j-1);
        Serial.print (" outTotal= ");
        Serial.print (adcOutTotal);
        Serial.println ();
*/
        } else {
          discardCounter++;
        }
      }
    }
    if(phaseCounter==0){ // the array is not full
      for (int i = 0; i<j; i++){
        if((adcInAverage+(ignoreThreshold*100)>=adcReading[i]) && 
           (adcInAverage-(ignoreThreshold*100)<=adcReading[i])){
          outCounter ++;  
          adcOutTotal+= adcReading[i];
          adcOutAverage = adcOutTotal/outCounter;
        } else {
          discardCounter++;
        }
      }
    }
/*    
    Serial.print ("WeMosReadVolts inCount= ");
    Serial.print (inCounter);
//    Serial.print (" phase= ");
//    Serial.print (phaseCounter);
    Serial.print (" outCount= ");
    Serial.print (outCounter);
    Serial.print (" discardCount= ");
    Serial.print (discardCounter);
//    Serial.print (" debugC1= ");
//    Serial.print (debugCounter1);
//    Serial.print (" debugC2= ");
//    Serial.print (debugCounter2);
    Serial.print (" adcAvgDelta= ");
    Serial.print ((long)(adcInAverage-adcOutAverage));
//    Serial.print (" adcOutAvg= ");
//    Serial.print (adcOutAverage);
    Serial.print (" adcTotdelta= ");
    Serial.print ((long)(adcInTotal-adcOutTotal));
//    Serial.print (" adcOutTot= ");
//    Serial.print (adcOutTotal);
    Serial.println ();
*/    
    return (((long)(adcOutAverage -zeroOffset)*(long)fullScale) / (1024-zeroOffset/100));
  }
}

// _____________________________
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
unsigned long WeMosVolts::filterAdc(unsigned long _adcCode, byte _filter){ //
//  filteredAdcCode = adcCode; // unfiltered (for testing)
//  static unsigned long filteredAdcCodeArray[filterLevel];
//      Serial.print ("filteredAdcCodeArray[0](before) ");
//      Serial.print (filteredAdcCodeArray[0]);
  if (_filter>10) _filter=10;
  unsigned long fa =filteredAdcCodeArray[0]<<_filter;
  filteredAdcCodeArray[0] = (fa + _adcCode-filteredAdcCodeArray[0])>>_filter;
  for (int i=1; i<filterLevel; i++){
    fa =filteredAdcCodeArray[i]<<_filter;
    filteredAdcCodeArray[i]= (fa + filteredAdcCodeArray[i-1]-filteredAdcCodeArray[i])>>_filter;
  } 
/*
      Serial.print (" AdcCode ");
      Serial.print (_adcCode);
      Serial.print (" filteredAdcCodeArray[0](after) ");
      Serial.print (filteredAdcCodeArray[0]);
      Serial.print (" _filter ");
      Serial.print (_filter);
      Serial.print (" filter ");
      Serial.print (filter);
      Serial.println ();
*/
  return filteredAdcCodeArray[filterLevel-1];
}

// _____________________________
void WeMosVolts::calibrateZero(){ 
  Serial.print ("calibrate Zero reading ");
  currentState = voltState_zero;
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
  
float WeMosVolts::getVolts(byte timeAllowed){ // returns Volts
  long _temp=0;
  return (float)getMilliVolts(timeAllowed)/1000;
}


void WeMosVolts::returnToNormal(){
  currentState = voltState_normal;
}


