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
#include "wemosPinMap.h" // class (or namespace) for wemos pin names

const int USBVcc = 4685 ; // mV - supply voltage from USB
const int arraySize = 200; // 32; // size of arrays for stats collection
const int delayBetweenMeasurements = 200; // ms
const unsigned int cyclePrintTrigger = 200; // print out stats
  // once every No. of measurement cycles
const int statsGroup = 1; // how many code values are grouped in one voltCode stats bin  
// const byte oversampling = 2;
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
  calState_waitScalethe Include Library 
} calState ;

LED ledV(redVPin, 1); // Vcc behind red LED
LED ledG(greenGPin, 0); // Ground behind green LED
// LED ledV(calibrateScalePin, 1); // Vcc behind red LED
// Bounce calZero=Bounce(calibrateZeroPin, 10); // bounce object: pin, ms
Bounce calZero(calibrateZeroPin, 10); // bounce object: pin, ms
// Bounce calZero(); // bounce object: pin, ms
Bounce calScale(calibrateScalePin, 10); // bounce object: pin, ms

void setup() {
  Serial.begin(115200);
  Serial.println("\n WeMosReadVolts");
  pinMode(calibrateZeroPin,INPUT);
  pinMode(calibrateScalePin,INPUT);
//  calZero.attach(calibrateZeroPin,INPUT);
  
  Serial.print ("WeMosVolts.voltsTime set to ");
  Serial.println (WeMosVolts.voltsTime);
  Serial.print ("WeMosVolts.taskPeriod is ");
  Serial.print (WeMosVolts.taskPeriod);
  Serial.println (" ms");
  
  delay(500);
  Serial.println("Initialising statistics counters and start values");
  initialiseStats();
  
//  Serial.println("delaying 13 s");
//  delay(13000);
  while (millis()>taskTime){ // increment start time to just after now
    taskTime += delayBetweenMeasurements;
  }
  Serial.print(" taskTime set to ");
  Serial.println(taskTime);
  ledG.on();
}

void loop() {
  WeMosVolts .run();
  // check Zero calibration button
  calZero.update(); // update status of Bounce object for Zero button
  zeroButtonRun();
  
  calScale.update(); // update status of Bounce object for Scale button
  scaleButtonRun();

  readVoltageRun();
}


