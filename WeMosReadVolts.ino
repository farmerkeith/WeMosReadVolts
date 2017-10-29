// test read Vcc
// ADC_MODE(ADC_VCC); // use with ESP.getVcc()
// WeMos D1 mini pro has a resistive divider on board, nominal values are
// Rhi = 220K Rlo = 100K. This will convert nominal range which is 1000 mV full scale to 
// 3200 mV full scale, plus any variation due to resistor tolerances.
// input impedance will be 320 K plus variation due to resistor tolerances. 
// to get full scale of 5.9 V put a 270K resistor in series with A0. 
// this is an example sketch to calibrate and read voltages

#include "WeMosReadVolts.h" // associated tab file
#include "farmerkeith_LED.h" // tab file

const int fullScale = 6230; // mV
const byte calibrateZeroPin  = 13; // D7
const byte calibrateScalePin = 4; // D2
const byte redVPin = 14; // set pin D5/GPIO14 for the red LED to Vcc
const byte greenGPin = 12; // set pin D6/GPIO12 for the green LED to Ground

int baseVoltage = 0; // mV, base for stats collection
int baseMeasureTime = 0; // mV, base for stats collection
const int arraySize = 100; // 32; // size of arrays for stats collection
const int delayBetweenMeasurements = 200; // ms
unsigned int cycleCounter = 0; // counting measurements
const unsigned int cyclePrintTrigger = 100; // print out stats
  // once every No. of measurement cycles
int rawVoltage;
int voltage, lowVoltage, highVoltage;
int value[arraySize]; // array for collecting voltage statistics
int mValue[arraySize]; // array for collecting measurement count statistics
int lowMean=0, highMean=0; // filtered values of low group and high group
int loHiBreak, loHiBreak1; // used for classifying measurements into Lo and Hi groups
const int filter=16; // exponential decay digital filter parameter
unsigned long measureTime, taskTime=0;
int mCount=0;

WeMosVolts wmv;
LED ledV(redVPin, 1); // Vcc behind red LED
LED ledG(greenGPin, 0); // Ground behind green LED
// LED ledV(calibrateScalePin, 1); // Vcc behind red LED


void setup() {
  Serial.begin(115200);
  Serial.println("\n WeMosReadVoltsTest");
  pinMode(calibrateZeroPin,INPUT);
  pinMode(calibrateScalePin,INPUT); // D2
//  pinMode(15,INPUT); // GPIO 15 is D8
//  pinMode(5,INPUT); // GPIO 5 is D1
//  pinMode(4,INPUT); // GPIO 4 is D2
//  pinMode(0,INPUT); // GPIO 0 is D3
//  pinMode(2,INPUT); // GPIO 2 is D4

  for (int i=0; i<arraySize; i++){
    value[i]=0;
    mValue[i] =0;
    voltage = readVoltage(1, 100000, rawVoltage, mCount, measureTime);
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
  // ESP.getVcc(); // can only be used if A0 is floating, cannot be used with WeMos D1 
  if (millis()>=taskTime){ // does not work when millis() rolls over, not a problem for short runs
    
    cycleCounter ++;
    voltage = readVoltage(1, 100000, rawVoltage, mCount, measureTime);
  
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
//    Serial.print (" lowMean ");
//    Serial.print ((float)lowMean/100);
//    Serial.print (" highMean ");
//    Serial.print ((float)highMean/100);
//    Serial.print (" lowV ");
//    Serial.print ((float)lowVoltage/1000,3);
    Serial.print (" ZeroPin=");
    Serial.print (digitalRead(calibrateZeroPin));
    Serial.print (" ScalePin=");
    Serial.print (digitalRead(calibrateScalePin));
    Serial.print (" highV ");
    Serial.println ((float)highVoltage/1000,3);

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
  // voltage stats
  for (int i=0; i<arraySize; i++){
    Serial.print("rawV ");
    if (baseVoltage+i<100) Serial.print(" "); // formatting
    Serial.print (baseVoltage+i);
    Serial.print(" V ");

    Serial.print((float)(baseVoltage+i)*fullScale*2/1024000,3); // 11K : 11K
    Serial.print(" ");
    
    Serial.print(value[i]);
    Serial.print(" ");
    for (int j=0; j<value[i]; j++){
      Serial.print ("*");
      if (j>200) break;
    }
    Serial.println();
  }
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
  }

}
// _______________________

float readVoltage(int _Rhi, int _Rlo, int &_rawVoltage, int &_mCount, unsigned long &_measureTime){
  int _fullScale=fullScale;
//  _mCount = 0;
//  for(_mCount=0; _mCount<20; _mCount++) {
    _measureTime = micros();
    _rawVoltage = wmv.getMilliVolts() ; // was analogRead(A0);
    _measureTime = micros() - _measureTime;
//    if (_measureTime<80) break;
//    delay(50);
//  } 
  
  return (float)_rawVoltage *_fullScale*(_Rhi+_Rlo)/_Rlo/1024;
}

