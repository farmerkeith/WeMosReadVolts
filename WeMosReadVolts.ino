// test read Vcc
// ADC_MODE(ADC_VCC); // use with ESP.getVcc()
// WeMos D1 mini pro has a resistive divider on board, nominal values are
// Rhi = 220K Rlo = 100K. This will convert nominal range which is 1000 mV full scale to 
// 3200 mV full scale, plus any variation due to resistor tolerances.
// input impedance will be 320 K plus variation due to resistor tolerances. 
// to get full scale of 5.9 V put a 270K resistor in series with A0. 
// this is an example sketch to calibrate and read voltages
// 1 Nov 2017 adding functions to use calibration pushbuttons calZero and calScale
// 7 Nov 2017 

#include "WeMosReadVolts.h" // associated tab file
#include "farmerkeith_LED.h" // tab file
#include <Bounce2.h> // for debouncing switch inputs
#include "wemosPinMap.h" // namespace for wemos pin names

const int USBVcc = 4685 ; // mV - supply voltage from USB
const int arraySize = 200; // 32; // size of arrays for stats collection
const int delayBetweenMeasurements = 200; // ms
const unsigned int cyclePrintTrigger = 200; // print out stats
  // once every No. of measurement cycles
const byte oversampling = 2;
// const long fullScale = 617371; // mV*100
const byte calibrateZeroPin  = wemosPin.D7; // 13; // D7, normally LOW
const byte calibrateScalePin = wemosPin.D2; // 4; // D2, normally HIGH
const byte redVPin = wemosPin.D5; // 14; // set pin D5/GPIO14 for the red LED to Vcc
const byte greenGPin = wemosPin.D6; // 12; // set pin D6/GPIO12 for the green LED to Ground

long baseCode = 0; // adc code, base for stats collection
long baseVoltage = 0; // mV, corresponding to baseCode
long baseMeasureTime = 0; // microseconds, base for stats collection
unsigned int cycleCounter = 0; // counting measurements
unsigned long milliVolts;
long voltCode;
unsigned long voltCodeMean;
float rawVoltageMean = 0;
float voltage;
unsigned int lowVoltage, highVoltage;
int value[arraySize]; // array for collecting voltage statistics
int mVForCode[arraySize]; // array for collecting voltage statistics
int mValue[arraySize]; // array for collecting measurement count statistics
unsigned long lowMean=0, highMean=0; // filtered values of low group and high group
unsigned long loHiBreak, loHiBreak1; // used for classifying measurements into Lo and Hi groups
const int filter=16; // exponential decay digital filter parameter
unsigned long measureTime, taskTime=0;
int mCount=0;
enum calState {
  calState_normal,
  calState_zero,
  calState_scale,
  calState_waitZero,
  calState_waitScale
} calState ;

WeMosVolts wmv; // create wmv object
LED ledV(redVPin, 1); // Vcc behind red LED
LED ledG(greenGPin, 0); // Ground behind green LED
// LED ledV(calibrateScalePin, 1); // Vcc behind red LED
Bounce calZero(calibrateZeroPin, 10); // bounce object: pin, ms
Bounce calScale(calibrateScalePin, 10); // bounce object: pin, ms

void setup() {
  Serial.begin(115200);
  Serial.println("\n WeMosReadVolts");
  pinMode(calibrateZeroPin,INPUT);
  pinMode(calibrateScalePin,INPUT);

  wmv.oversampling = oversampling; 
  delay(500);
  Serial.println("Initialising statistics counters and start values");
  initialiseStats();
  
//  Serial.println("delaying 13 s");
//  delay(13000);
  while (millis()>taskTime){ // increment start time to just after now
    taskTime += delayBetweenMeasurements;
  }
  Serial.print("taskTime set to ");
  Serial.println(taskTime);
  ledG.on();
}

void loop() {
  // check calibration button states

  // Zero button 
  calZero.update(); // update status of Bounce object
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
      wmv.calibrateZero();
      Serial.print (" Zero calibration done");
      Serial.print (" zeroOffset=");
      Serial.print (wmv.zeroOffset);
      Serial.print (" Min=");
      Serial.print (wmv.zeroOffsetMin);
      Serial.print (" Max=");
      Serial.print (wmv.zeroOffsetMax);
      Serial.print (" Exp8=");
      Serial.print (wmv.zeroOffsetExp);
      Serial.print (" Delta=");
      Serial.println (wmv.zeroOffset-wmv.zeroOffsetExp);
      initialiseStats();
    } 
  }
  if (calState == calState_waitZero){
    if (ZeroButton==LOW) { // button released 
      Serial.println ("\nZero calibration button released");
      calState = calState_normal;
      printStats();
      initialiseStats();
    }
  }

// Scale button    
// ---------------------
  calScale.update();
  bool ScaleButton = calScale.read(); // calScale HIGH when button not pressed

  if (calState == calState_normal){
    if (ScaleButton==LOW) calState = calState_scale; // calScale button pressed
  }
  if (calState == calState_scale){
    if (ScaleButton==LOW) { // button pressed
      Serial.println ("\nScale calibration button pressed");
      calState = calState_waitScale;
//      printStats();
      delay(50);
      wmv.calibrateScale(USBVcc);
      Serial.print (" Scale calibration done");
      Serial.print (" fullScale=");
      Serial.println (wmv.fullScale);
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


// ---------------------  
  if (millis()>=taskTime){ // does not work when millis() rolls over, not a problem for short runs
    
    cycleCounter ++;
    milliVolts = readVoltage(voltCode, mCount, measureTime);

    // separate Low and High groups and generate filtered means
    loHiBreak = loHiBreak*(filter-1)/filter+ 100*milliVolts/filter;
    if (milliVolts - loHiBreak/100<0){
      lowMean = lowMean*(filter-1)/filter+ 100*milliVolts/filter;
    } else {
      highMean = highMean*(filter-1)/filter+ 100*milliVolts/filter;
    }
  
    lowVoltage = lowMean ; // *fullScale/102400; // 11K : 11K
    highVoltage = highMean ; // *fullScale/102400; // 11K : 11K

    printMeasureLine();

    ledV.toggle();
    ledG.toggle();

    collectStats();
    
    if (cycleCounter%cyclePrintTrigger==0){
      printStats();
      Serial.print  ("cycleCounter ");
      Serial.println(cycleCounter);
//      Serial.println();
    }
    taskTime += delayBetweenMeasurements;
  }
}

// _______________________
void collectStats(){
  rawVoltageMean += milliVolts;
  voltCodeMean += voltCode;
  int index = voltCode-baseCode;
//  int index = milliVolts-baseVoltage;
  if (index <0) index = 0;
  if (index > arraySize-1) index = arraySize-1;
  value[index]++;
  mVForCode[index] = milliVolts;
// mCount statistics
//  index = mCount;
//  if (index > arraySize-1) index = arraySize-1;
//  mValue[index]++;

// measureTime statistics
  index = measureTime - baseMeasureTime;
  if (index <0) index = 0;
  if (index > arraySize-1) index = arraySize-1;
  mValue[index]++;
}

// _______________________
void printStats(){
//  printMCountStats();
  printVoltageStats();
} // end of void printStats()
// _______________________

float readVoltage(long &_voltCode, int &_mCount, unsigned long &_measureTime){
    unsigned long _milliVolts;
//    long _temp=0;
//  long _fullScale=fullScale;
//  _mCount = 0;
//  for(_mCount=0; _mCount<20; _mCount++) {
    _measureTime = micros();
//    _rawVoltage = analogRead(A0);
    _milliVolts = wmv.getMilliVolts(_voltCode) ;
    _measureTime = micros() - _measureTime;
//    if (_measureTime<80) break;
//    delay(50);
//  } 
  
  return _milliVolts;
}

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

void  printVoltageStats(){
  Serial.println("\nVoltage stats");
  Serial.print ("Oversampling: ");
  Serial.print (oversampling);
  Serial.println (" measurements per value");
  Serial.println ("Delay in oversampling loop = 20ms");
  Serial.println ("VCode VCode/os mV   Count  % Graphical");
  
  int maxValue = 0;
  for (int i=0; i<arraySize; i++){
    if (value[i]> maxValue) maxValue = value[i];
  }
  if (maxValue <80) maxValue = 80;
  for (int i=0; i<arraySize; i++){
    if(value[i]!=0||value[i-1]!=0||value[i+1]!=0){
//      Serial.print("VCode ");
      if (baseCode+i<100) Serial.print(" "); // formatting
      Serial.print (baseCode+i);
      Serial.print(" ");
      Serial.print ((float)(baseCode+i)/oversampling,2);
      Serial.print(" ");
//      Serial.print(" V ");
      Serial.print((float)mVForCode[i]/1000,3); 
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
  printMeanVoltage();
  
  rawVoltageMean = 0;
}

void initialiseStats(){
  baseCode=0;
  baseVoltage=0;
  baseMeasureTime=0;
  for (int i=0; i<arraySize; i++){
    value[i]=0;
    mValue[i] =0;
  }
  for (int i=0; i<8; i++){
    milliVolts = readVoltage(voltCode, mCount, measureTime);
//    int tempVoltage = analogRead(A0);
    baseCode += voltCode;
    baseVoltage += milliVolts; // getting a base value for stats collection
    baseMeasureTime += measureTime;     
//    delay(1);
  }
  
  baseVoltage = baseVoltage/8;
  baseMeasureTime = baseMeasureTime/8 - arraySize/4;
  lowMean = baseVoltage * 100; // initialise close to final
  highMean = lowMean; // initialise close to final
  loHiBreak = lowMean; // initialise close to final
  baseVoltage = baseVoltage -(arraySize / 2);
  if (baseVoltage<0) baseVoltage = 0;
  baseCode = baseCode/8 -(arraySize / 2);
  if (baseCode<0) baseCode = 0;
  Serial.print("Base Voltage set to ");
  Serial.print(baseVoltage);
  Serial.print(" Base Code set to ");
  Serial.print(baseCode);
  Serial.print(" equivalent ");
  Serial.println((float)baseCode/oversampling);
  Serial.print(" lowMean, highMean and loHiBreak set to ");
  Serial.println((float)lowMean/100);
}

void printMeasureLine(){
    Serial.print ("T ");
    Serial.print ((float)millis()/1000,3);
    Serial.print (" voltCode/ov ");
    Serial.print ((float)voltCode/oversampling);
    Serial.print (" V ");
    Serial.print ((float)milliVolts/1000,3);
//    Serial.print (" rawV ");
//    Serial.print(rawVoltage);

//    Serial.print (" measureTime=");
//    Serial.print (measureTime);
//    Serial.print (" mCount=");
//    Serial.print (mCount);

  Serial.print (" Break ");
  Serial.print (loHiBreak/100);
//  Serial.print (" loHiBreak1 ");
//  Serial.print (loHiBreak1/10);
//    Serial.print (" raw-Break ");
//    Serial.print (rawVoltage - loHiBreak/100);
    Serial.print (" lowRawMean ");
    Serial.print ((float)lowMean/100);
    Serial.print (" highRawMean ");
    Serial.print ((float)highMean/100);
    Serial.print (" lowV ");
    Serial.print ((float)lowVoltage/100,3);
//    Serial.print (" ZeroPin=");
//    Serial.print (digitalRead(calibrateZeroPin));
//    Serial.print (" ScalePin=");
//    Serial.print (digitalRead(calibrateScalePin));
    Serial.print (" highV ");
    Serial.print ((float)highVoltage/100,3);
    Serial.println();
}

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
  Serial.println((float)VMean/iTot,4);
}
