// File created 9 Nov 2017

// _______________________
void collectStats(){
// accumulate totals for averages  
  rawVoltageMean += milliVoltsG;
  voltCodeMean += voltCode;
  if (milliVoltsG<lowVoltage) lowVoltage=milliVoltsG;
  if (milliVoltsG>highVoltage) highVoltage=milliVoltsG;
  
// collect voltCode statistics
//  int index = (voltCode-baseCode)/statsGroup; 
  
// collect voltage statistics
  int index = ((long)(milliVoltsG-baseVoltage))/statsGroup;
//  Serial.print ("Stats index= ");
//  Serial.println (index);
  if (index <0) index = 0;
  if (index > arraySize-1) index = arraySize-1;
  value[index]++;
  mVForCode[index] = milliVoltsG;
// mCount statistics
//  index = mCount;
//  if (index > arraySize-1) index = arraySize-1;
//  mValue[index]++;

// collect measureTime statistics
  index = measureTime - baseMeasureTime;
  if (index <0) index = 0;
  if (index > arraySize-1) index = arraySize-1;
  mValue[index]++;
}

// _______________________
void initialiseStats(){
  baseCode=0;
  baseVoltage=0;
  baseMeasureTime=0;
  lowVoltage= 10000000;
  highVoltage= 0;

  for (int i=0; i<arraySize; i++){
    value[i]=0;
    mValue[i] =0;
    mVForCode[i]=0;
  }
  for (int i=0; i<8; i++){
    milliVoltsG = readVoltage(voltCode, mCount, measureTime);
//    int tempVoltage = analogRead(A0);
//    Serial.print(" initialising, measured milliVolts ");
//    Serial.println(milliVoltsG);
    baseCode += voltCode;
    baseVoltage += milliVoltsG; // getting a base value for stats collection
    baseMeasureTime += measureTime;     
//    delay(1);
  }

//  Serial.print(" measured Volt Code ");
//  Serial.println(baseCode/8);
  Serial.print(" measured milliVolts ");
  Serial.println((float)baseVoltage/800);

  baseVoltage = baseVoltage/8;
  baseMeasureTime = baseMeasureTime/8 - arraySize/4;
  lowMean = baseVoltage * 100; // initialise close to final
  highMean = lowMean; // initialise close to final
  loHiBreak = lowMean; // initialise close to final
  baseVoltage = baseVoltage -(arraySize*statsGroup / 2);
  if (baseVoltage<0) baseVoltage = 0;
  baseCode = (baseCode/8) -(arraySize*statsGroup / 2);
  if (baseCode<0) baseCode = 0;
  if (initialisePrinting){
    Serial.println("Stats counters re-initialised");
    Serial.print("Base Voltage set to ");
    Serial.print(baseVoltage);
    Serial.print(" Base Code set to ");
    Serial.print(baseCode);
    Serial.print(" lowMean, highMean and loHiBreak set to ");
    Serial.println((float)lowMean/100);
  }
  Serial.println();
}

// _______________________
void printMCountStats(){
//  for (int i=0; i<21; i++){
  for (int i=0; i<arraySize; i++){
    Serial.print("measureTime ");
    Serial.print (baseMeasureTime + i);
    Serial.print(" Count ");

    Serial.print(mValue[i]);
    Serial.print(" ");
    for (int j=0; j<mValue[i]; j++){
      Serial.print ("*");
      if (j>200) break;
    }
    Serial.println();
  } // end of for (int i=0; i<arraySize; i++)
}

// _______________________
void printMeanVoltage(){
  unsigned long VMean=0, iTot=0;
  for (int i=0; i<arraySize; i++){
      VMean += value[i]*(mVForCode[i]);
      iTot += value[i];
  }
  Serial.print("Counts ");
  Serial.print(iTot);
  Serial.print(" Aggregate voltage ");
  Serial.print(VMean);
  Serial.print(" Mean milli volts ");
  Serial.println((float)VMean/iTot/100,4);
}

// _______________________
void printMeasureLine(){
    Serial.print ("T ");
    Serial.print ((float)millis()/1000,3);
    Serial.print (" voltCode ");
    Serial.print (voltCode);
    Serial.print (" V ");
    Serial.print ((float)milliVoltsG/100,2);
    Serial.print (" delta ");
    if (milliVoltsG > baseVoltage) Serial.print(" ");
    if (milliVoltsG+1000 > baseVoltage) Serial.print(" ");
    if (milliVoltsG-1000 < baseVoltage) Serial.print(" ");
    Serial.print ((float)((long)(milliVoltsG - baseVoltage))/100);
    
//    Serial.print("  ");
//    Serial.print((long)(milliVolts - baseVoltage));
//    Serial.print ((long)(voltCode - baseCode));
    int index1=0, index2=0;
    index1 = ((long)(milliVoltsG - baseVoltage)/100)%50;
    index2 = ((long)(milliVoltsG - baseVoltage)/100)/50;
//    index1 = (voltCode - baseCode)%50;
//    index2 = (voltCode - baseCode)/50;
//    Serial.print (" ");
//    Serial.print (index1);


    Serial.print (" ");
    for (int i=0; i<index1+25; i++){
      if(index2==4) Serial.print ("*");
      else if(index2==3) Serial.print ("@");
      else if(index2==5) Serial.print ("#");
      else if(index2==1) Serial.print ("$");
      else if(index2==0) Serial.print ("&");
      else if(index2==-1) Serial.print ("+");
      else if(index2==2) Serial.print ("^");
      else Serial.print (".");
    }
//    Serial.print (" rawV ");
//    Serial.print(rawVoltage);

//    Serial.print (" measureTime=");
//    Serial.print (measureTime);
//    Serial.print (" mCount=");
//    Serial.print (mCount);

//  Serial.print (" Break ");
//  Serial.print (loHiBreak/100);
//  Serial.print (" loHiBreak1 ");
//  Serial.print (loHiBreak1/10);
//    Serial.print (" raw-Break ");
//    Serial.print (rawVoltage - loHiBreak/100);
//    Serial.print (" lowRawMean ");
//    Serial.print ((float)lowMean/100);
//    Serial.print (" highRawMean ");
//    Serial.print ((float)highMean/100);
//    Serial.print (" lowV ");
//    Serial.print ((float)lowVoltage/100,3);
//    Serial.print (" ZeroPin=");
//    Serial.print (digitalRead(calibrateZeroPin));
//    Serial.print (" ScalePin=");
//    Serial.print (digitalRead(calibrateScalePin));
//    Serial.print (" highV ");
//    Serial.print ((float)highVoltage/100,3);
    Serial.println();
}

// _______________________
void printStats(){
  if (statsMCountPrinting)  printMCountStats();
  if (statsVoltagePrinting) printVoltageStats();
  if (rangeVoltagePrinting) printVoltageRange();
  if (meanVoltagePrinting)  printMeanVoltage();
  if (configurationPrinting) printConfiguration();
  
} // end of void printStats()

void printConfiguration(){
  Serial.print("WeMosVolts config:");
  Serial.print (" taskPeriod "); Serial.print(WeMosVolts.taskPeriod);
  Serial.print (" filter "); Serial.print(WeMosVolts.filter);
  Serial.print (" filterLevel "); Serial.print(WeMosVolts.filterLevel);
  Serial.print (" ignoreThreshold "); Serial.print(WeMosVolts.ignoreThreshold);
  Serial.print (" timeAllowed "); Serial.println(WeMosVolts.timeAllowed);

}

// _______________________
void  printVoltageStats(){
  Serial.println("\nVoltage stats");
  Serial.print("baseCode ");
  Serial.print(baseCode);
  Serial.print(" baseVoltage ");
  Serial.print((float)baseVoltage/100);
  Serial.print(" maxVoltage ");
  Serial.println((float)(baseVoltage+(arraySize-1)*statsGroup)/100);
  
  Serial.println ("VCode    mV   Count  % Graphical");
  
  int maxValue = 0;
  for (int i=0; i<arraySize; i++){
    if (value[i]> maxValue) maxValue = value[i];
  }
  if (maxValue <80) maxValue = 80;
  for (int i=0; i<arraySize; i++){
    if(value[i]!=0||value[i-1]!=0||value[i+1]!=0){
//      Serial.print("VCode ");
      if (baseVoltage+i<100) Serial.print(" "); // formatting
//      if (baseCode+i<100) Serial.print(" "); // formatting
      Serial.print (baseVoltage+i*statsGroup); 
//      Serial.print (baseCode+i*statsGroup); 
//      Serial.print(" ");
//      Serial.print ((float)(baseCode+i)/oversampling,2);
      Serial.print(" ");
//      Serial.print(" V ");
      if(mVForCode[i]<10) Serial.print (" ");
      if(mVForCode[i]<100) Serial.print (" ");
      if(mVForCode[i]<1000) Serial.print (" ");
      Serial.print((float)mVForCode[i]/100,2); 
      Serial.print(" ");
      if(value[i]<10) Serial.print (" ");
      if(value[i]<100) Serial.print (" ");
      if(value[i]<1000) Serial.print (" ");
      Serial.print(value[i]); // raw count
      Serial.print(" ");
      float v1 = ((float)value[i]*100/maxValue); // normalise
      if(v1<10) Serial.print (" ");
      if(v1<100) Serial.print (" ");
      if(v1<1000) Serial.print (" ");
      Serial.print(v1,1); // normalised
      if (value[i] != 0){
        Serial.print(" ");
        int j1 = value[i]*80/maxValue;
        if (j1==0) Serial.print ("!");
        else {
          for (int j=0; j<j1; j++){
          Serial.print ("*");
          if (j>200) break;
          }
        }
      }
      Serial.println();
    }
  }
  
  
//    if (milliVolts>highVoltage) highVoltage=milliVolts;

  rawVoltageMean = 0;
}


// _______________________
void printVoltageRange(){
  Serial.print(" cycleCounter ");
  Serial.print(cycleCounter);
  Serial.print (" lowVoltage ");
  Serial.print ((float)lowVoltage/100);
  Serial.print (" highVoltage ");
  Serial.print ((float)highVoltage/100);
  Serial.print (" Voltage range ");
  Serial.println ((float)(highVoltage - lowVoltage)/100);
}

// _______________________
float readVoltage(long &_voltCode, int &_mCount, unsigned long &_measureTime){
    unsigned long _milliVolts;
//    long _temp=0;
//  long _fullScale=fullScale;
//  _mCount = 0;
//  for(_mCount=0; _mCount<20; _mCount++) {
    _measureTime = micros();
//    _rawVoltage = analogRead(A0); // for testing
    _milliVolts = WeMosVolts.getMilliVolts(timeAllowedG) ;
//    Serial.println ("just got milli volts Functions line 290");
    _voltCode = WeMosVolts.getVoltCode(timeAllowedG) ;
    _measureTime = micros() - _measureTime;
//    if (_measureTime<80) break;
//    delay(50);
//  } 
  
  return _milliVolts;
}

// _______________________
void  readVoltageRun(){
  if ((long)(millis() - taskTime)>=0){ 
    taskTime += delayBetweenMeasurements; // set next execution time
    cycleCounter ++;
    milliVoltsG = readVoltage(voltCode, mCount, measureTime);
//    // separate Low and High groups and generate filtered means
//    loHiBreak = loHiBreak*(filter-1)/filter+ 100*milliVolts/filter;
//    if (milliVolts - loHiBreak/100<0){
//      lowMean = lowMean*(filter-1)/filter+ 100*milliVolts/filter;
//    } else {
//      highMean = highMean*(filter-1)/filter+ 100*milliVolts/filter;
//    }
//    lowVoltage = lowMean ; // *fullScale/102400; // 11K : 11K
//    highVoltage = highMean ; // *fullScale/102400; // 11K : 11K

    if (eachReading){
      if (cycleCounter%eachReading==0) printMeasureLine(); 
    }

    ledV.toggle();
    ledG.toggle();

    collectStats();
    
//    if (statsPrinting){
//      Serial.print  ("cycleCounter ");
//      Serial.println(cycleCounter);
//    }
    if (cycleCounter%cyclePrintTrigger==0){
      printStats();
      Serial.print (" cycleCounter%(cyclePrintTrigger*initialiseTrigger)= ");
      Serial.print (cycleCounter%(cyclePrintTrigger*initialiseTrigger));
      Serial.print (" will re-initialise at ");
      Serial.println (cyclePrintTrigger*initialiseTrigger);
      if (initialiseTrigger){
        if (cycleCounter%(cyclePrintTrigger*initialiseTrigger)==0){
          initialiseStats();
          filterG++; 
          if (filterG==11){ 
            filterG=0;
            filterLevelG++;
            if (filterLevelG==9) filterLevelG=1;
          }
          WeMosVolts.filter=filterG; // exponential decay digital filter parameter (Mode 1)
          WeMosVolts.filterLevel = filterLevelG ; // No. of filter stages in series (max is 8) (Mode 1)
          Serial.print("filtering updated, filter=");
          Serial.print(filterG);
          Serial.print(" filter Level=");
          Serial.println(filterLevelG);
        }
      }
    }
  }
}

// _______________________
void zeroButtonRun(){
  bool ZeroButton = calZero.read(); // calZero LOW when button not pressed
  if (calState == calState_normal){
    if (ZeroButton==HIGH) calState = calState_zero; // calZero button pressed
  }
  if (calState == calState_zero){
    if (ZeroButton==HIGH) { // button pressed
      Serial.println ("\nZero calibration button pressed");
      calState = calState_waitZero;
      printStats();
      delay(50);
      WeMosVolts .calibrateZero(); // initial zero calibration
      Serial.print (" Zero calibration done");
      Serial.print (" zeroOffset=");
      Serial.print (WeMosVolts .zeroOffset);
      Serial.print (" Min=");
      Serial.print (WeMosVolts .zeroOffsetMin);
      Serial.print (" Max=");
      Serial.print (WeMosVolts .zeroOffsetMax);
      Serial.print (" Exp8=");
      Serial.print (WeMosVolts .zeroOffsetExp);
      Serial.print (" Delta=");
      Serial.println (WeMosVolts .zeroOffset-WeMosVolts .zeroOffsetExp);
      initialiseStats();
    } 
  }
  if (calState == calState_waitZero){
    if (ZeroButton==HIGH) { // button pressed
      if (WeMosVolts.milliVolts <0) WeMosVolts .calibrateZero(); // re-calibrate
    }
    if (ZeroButton==LOW) { // button released 
      Serial.println ("\nZero calibration button released");
      calState = calState_normal;
      WeMosVolts .returnToNormal();
      printStats();
      initialiseStats();
    }
  }
}

// _______________________
void  scaleButtonRun(){
  bool ScaleButton = calScale.read(); // calScale HIGH when button not pressed

  if (calState == calState_normal){
    if (ScaleButton==LOW) calState = calState_scale; // calScale button pressed
  }
  if (calState == calState_scale){
    if (ScaleButton==LOW) { // button pressed
      Serial.println ("\nScale calibration button pressed");
      calState = calState_waitScale;
      printStats();
      delay(50);
      WeMosVolts.calibrateScale(USBVcc);
      Serial.print (" Scale calibration done");
      Serial.print (" fullScale=");
      Serial.println (WeMosVolts.fullScale);
      initialiseStats();
    } 
  }
  if (calState == calState_waitScale){
    if (ScaleButton==HIGH) { // button released 
      Serial.println ("\nScale calibration button released");
      calState = calState_normal;
      printStats();
      Serial.print ("reference voltage USBVcc =");
      Serial.println (USBVcc);
      initialiseStats();
    }
  }
}



