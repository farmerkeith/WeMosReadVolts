// test read Vcc
// ADC_MODE(ADC_VCC); // use with ESP.getVcc()
// WeMos D1 mini pro has a resistive divider on board, nominal values are
// Rhi = 220K Rlo = 100K. This will convert nominal range which is 1000 mV full scale to 
// 3200 mV full scale, plus any variation due to resistor tolerances.
// input impedance will be 320 K plus variation due to resistor tolerances. 
// to get full scale of 5.9 V put a 270K resistor in series with A0. 
// this is an example sketch to calibrate and read voltages
// 1 Nov 2017 adding functions to use calibration pushbuttons calZero and calScale

#include "WeMosReadVolts.h" // associated tab file
#include "farmerkeith_LED.h" // tab file
#include <Bounce2.h> // for debouncing switch inputs

const int USBVcc = 4720 ; // mV - supply voltage from USB
const long fullScale = 617371; // mV*100
const byte calibrateZeroPin  = 13; // D7, normally LOW
const byte calibrateScalePin = 4; // D2, normally HIGH
const byte redVPin = 14; // set pin D5/GPIO14 for the red LED to Vcc
const byte greenGPin = 12; // set pin D6/GPIO12 for the green LED to Ground

int baseVoltage = 0; // mV, base for stats collection
int baseMeasureTime = 0; // microseconds, base for stats collection
const int arraySize = 100; // 32; // size of arrays for stats collection
const int delayBetweenMeasurements = 200; // ms
unsigned int cycleCounter = 0; // counting measurements
const unsigned int cyclePrintTrigger = 100; // print out stats
  // once every No. of measurement cycles
long rawVoltage;
float rawVoltageMean = 0;
float voltage;
int lowVoltage, highVoltage;
int value[arraySize]; // array for collecting voltage statistics
int mValue[arraySize]; // array for collecting measurement count statistics
int lowMean=0, highMean=0; // filtered values of low group and high group
int loHiBreak, loHiBreak1; // used for classifying measurements into Lo and Hi groups
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
  Serial.println("\n WeMosReadVoltsTest");
  pinMode(calibrateZeroPin,INPUT);
  pinMode(calibrateScalePin,INPUT);

  for (int i=0; i<arraySize; i++){
    value[i]=0;
    mValue[i] =0;
    voltage = readVoltage(rawVoltage, mCount, measureTime);
//    int tempVoltage = analogRead(A0);
    baseVoltage += rawVoltage; // getting a base value for stats collection
    baseMeasureTime += measureTime;     
//    Serial.print("Setup raw voltage reading ");
//    Serial.println(tempVoltage);
    delay(50);
//    ledG.blink(200,10);

  }
  
  baseVoltage = baseVoltage/arraySize;
  baseMeasureTime = baseMeasureTime/arraySize-(arraySize / 4);
  lowMean = baseVoltage * 100; // initialise close to final
  highMean = lowMean; // initialise close to final
  loHiBreak = lowMean; // initialise close to final
  baseVoltage = baseVoltage -(arraySize / 2); // -5 removed
  Serial.print("Base Voltage set to ");
  Serial.println(baseVoltage);
  Serial.print(" lowMean, highMean and loHiBreak set to ");
  Serial.println((float)lowMean/100);
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
  calZero.update();
  bool ZeroButton = calZero.read(); // calZero LOW when button not pressed
  if (calState == calState_normal){
    if (ZeroButton==HIGH) calState = calState_zero; // calZero button pressed
  }
  if (calState == calState_zero){
    if (ZeroButton==HIGH) { // button pressed
      calState = calState_waitZero;
      wmv.calibrateZero();
      Serial.print (" Zero calibration done");
      Serial.print (" zeroOffset=");
      Serial.println (wmv.zeroOffset);
    } 
  }
  if (calState == calState_waitZero){
    if (ZeroButton==LOW) { // button released 
      calState = calState_normal;
      Serial.println (" Zero button released 113");
    }
  }
// same thing for scale button    
// ---------------------
  calScale.update();
  bool ScaleButton = calScale.read(); // calScale HIGH when button not pressed

  if (calState == calState_normal){
    if (ScaleButton==LOW) calState = calState_scale; // calScale button pressed
  }
  if (calState == calState_scale){
    if (ScaleButton==LOW) { // button pressed
      calState = calState_waitScale;
      wmv.calibrateScale(USBVcc);
      Serial.print (" Scale calibration done");
      Serial.print (" fullScale=");
      Serial.println (wmv.fullScale);
    } 
  }
  if (calState == calState_waitScale){
    if (ScaleButton==HIGH) { // button released 
      calState = calState_normal;
      Serial.println (" Scale button released 136");
    }
  }


// ---------------------  
  if (millis()>=taskTime){ // does not work when millis() rolls over, not a problem for short runs
    
    cycleCounter ++;
    voltage = readVoltage(rawVoltage, mCount, measureTime);
  
    Serial.print ("T ");
    Serial.print ((float)millis()/1000,3);
    Serial.print (" TT ");
    Serial.print ((float)taskTime/1000,3);
    Serial.print (" V ");
    Serial.print ((float)voltage/1000,3);
    Serial.print (" rawV ");
    Serial.print(rawVoltage);

    Serial.print (" measureTime=");
    Serial.print (measureTime);
//    Serial.print (" mCount=");
//    Serial.print (mCount);

    // separate Low and High groups and generate filtered means
    loHiBreak = loHiBreak*(filter-1)/filter+ 100*rawVoltage/filter;
    if (rawVoltage - loHiBreak/100<0){
      lowMean = lowMean*(filter-1)/filter+ 100*rawVoltage/filter;
    } else {
      highMean = highMean*(filter-1)/filter+ 100*rawVoltage/filter;
    }
  
    lowVoltage = lowMean *fullScale/102400; // 11K : 11K
    highVoltage = highMean *fullScale/102400; // 11K : 11K

//  Serial.print (" Break ");
//  Serial.print (loHiBreak/100);
//  Serial.print (" loHiBreak1 ");
//  Serial.print (loHiBreak1/10);
//    Serial.print (" raw-Break ");
//    Serial.print (rawVoltage - loHiBreak/100);
    Serial.print (" lowRawMean ");
    Serial.print ((float)lowMean/100);
    Serial.print (" highRawMean ");
    Serial.print ((float)highMean/100);
    Serial.print (" lowV ");
    Serial.print ((float)lowVoltage/1000,3);
//    Serial.print (" ZeroPin=");
//    Serial.print (digitalRead(calibrateZeroPin));
//    Serial.print (" ScalePin=");
//    Serial.print (digitalRead(calibrateScalePin));
    Serial.print (" highV ");
    Serial.print ((float)highVoltage/1000,3);
    Serial.println();

    ledV.toggle();
    ledG.toggle();

    collectStats();
    
    if (cycleCounter%cyclePrintTrigger==0){
      printStats();
      Serial.print  ("cycleCounter ");
      Serial.println(cycleCounter);
      Serial.println();
    }
    taskTime += delayBetweenMeasurements;
  }
}

// _______________________
void collectStats(){
  rawVoltageMean += rawVoltage;
  int index = rawVoltage-baseVoltage;
  if (index <0) index = 0;
  if (index > arraySize-1) index = arraySize-1;
  value[index]++;
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
  // mCount stats
//  for (int i=0; i<21; i++){
  for (int i=0; i<arraySize; i++){
//    Serial.print("mCount ");
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
  
  // voltage stats
  for (int i=0; i<arraySize; i++){
    Serial.print("rawV ");
    if (baseVoltage+i<100) Serial.print(" "); // formatting
    Serial.print (baseVoltage+i);
    Serial.print(" V ");

    Serial.print((float)(baseVoltage+i)*fullScale/1024000,3); // 11K : 11K
    Serial.print(" ");
    
    Serial.print(value[i]);
    Serial.print(" ");
    for (int j=0; j<value[i]; j++){
      Serial.print ("*");
      if (j>200) break;
    }
    Serial.println();
  }
// print mean voltage

  rawVoltageMean /= cyclePrintTrigger;
  Serial.print ("raw voltage mean ");
  Serial.println (rawVoltageMean,4);
  rawVoltageMean = 0;


} // end of void printStats()
// _______________________

float readVoltage(long &_rawVoltage, int &_mCount, unsigned long &_measureTime){
  long _fullScale=fullScale;
//  _mCount = 0;
//  for(_mCount=0; _mCount<20; _mCount++) {
    _measureTime = micros();
//    _rawVoltage = analogRead(A0);
    _rawVoltage = wmv.getMilliVolts() ; // was analogRead(A0);
    _measureTime = micros() - _measureTime;
//    if (_measureTime<80) break;
//    delay(50);
//  } 
  
  return _rawVoltage;
}

