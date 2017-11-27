// test read Vcc
// ADC_MODE(ADC_VCC); // use with ESP.getVcc() not available with WeMos D1 mini pro
// WeMos D1 mini pro has a resistive divider on board, nominal values are
// Rhi = 220K Rlo = 100K. This will convert nominal range which is 1000 mV full scale to 
// 3200 mV full scale, plus any variation due to resistor tolerances.
// input impedance will be 320 K plus variation due to resistor tolerances. 
// to get full scale of 5.9 V put a 270K resistor in series with A0. 
// this is an example sketch to calibrate and read voltages
// 1 Nov 2017 adding functions to use calibration pushbuttons calZero and calScale
// 2 - 23 Nov 2017 Added statistics collection; LEDs to show running status; 2 models of reading
// 25 Nov updated WeMosReadVolts class for 2 modes of operation, continuous and single shot

#include "WeMosReadVolts.h" // associated tab file
#include "farmerkeith_LED.h" // tab file
#include <Bounce2.h> // for debouncing switch inputs
#include "wemosPinMap.h" // class (or namespace) for wemos pin names

// configuration data for measuring
const int USBVcc = 4685 ; // mV - supply voltage from USB
const int arraySize = 200; // 32; // size of arrays for stats collection
const int delayBetweenMeasurements = 200; // ms
const unsigned int cyclePrintTrigger = 200; // print out stats
  // once every No. of measurement cycles
const int statsGroup = 20; // how many code values are grouped in one voltCode stats bin  
const int initialiseTrigger=10; // how many stats prints before re-initialising

// configuration data for class WeMosReadVolts
const int taskPeriodG = 20; // interval between Mode 1 measurements (default: 20 ms)
int filterG=0; // exponential decay digital filter parameter (Mode 1, value 0 to 10)
byte filterLevelG = 8; // No. of filter stages in series (max is 8) (Mode 1)
const int ignoreThresholdG=2; // discard ADC readings if (ADC Reading-Average) exceeds this (Modes 1 and 2)
const byte timeAllowedG=0; // 0 for Mode 1 (+run), otherwise Mode 2
const byte discardLimitG=3; // maximum number of discards in a row (mode 1 only)

// Serial print controls
const byte eachReading=10; // prints a line per n readings if n, no printing if 0
const bool statsMCountPrinting=0;
const bool statsVoltagePrinting=1;
const bool rangeVoltagePrinting=1;
const bool meanVoltagePrinting=1;
const bool initialisePrinting=1; // print result of initialiseStats()
const bool configurationPrinting=1;

// hardware configuration
const byte calibrateZeroPin  = wemosPin.D7; // 13; // D7, normally LOW
const byte calibrateScalePin = wemosPin.D2; // 4; // D2, normally HIGH
const byte redVPin = wemosPin.D5; // 14; // set pin D5/GPIO14 for the red LED to Vcc
const byte greenGPin = wemosPin.D6; // 12; // set pin D6/GPIO12 for the green LED to Ground

// variables
long baseCode = 0; // adc code, base for stats collection
long baseVoltage = 0; // mV, corresponding to baseCode
long baseMeasureTime = 0; // microseconds, base for stats collection
unsigned int cycleCounter = 0; // counting measurements
unsigned long milliVoltsG;
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
unsigned long measureTime, taskTime=0;
int mCount=0;
enum calState { // calibration state variable
  calState_normal,
  calState_zero,
  calState_scale,
  calState_waitZero,
  calState_waitScale,
} calState ;

// instantiate objects
LED ledV(redVPin, 1); // Vcc behind red LED
LED ledG(greenGPin, 0); // Ground behind green LED
// LED ledV(calibrateScalePin, 1); // Vcc behind red LED
// Bounce calZero=Bounce(calibrateZeroPin, 10); // bounce object: pin, ms
Bounce calZero(calibrateZeroPin, 10); // bounce object: pin, ms // Note: this usage is deprecated
// Bounce calZero(); // bounce object: pin, ms // note: trouble getting this to work!
Bounce calScale(calibrateScalePin, 10); // bounce object: pin, ms

void setup() {
  Serial.begin(115200);
  Serial.println("\n WeMosReadVolts");

// set up pins for Bounce library
  pinMode(calibrateZeroPin,INPUT);
  pinMode(calibrateScalePin,INPUT);
//  calZero.attach(calibrateZeroPin,INPUT);

// configure object WeMosReadVolts
  WeMosVolts.taskPeriod = taskPeriodG; // interval between Mode 1 measurements (default: 20 ms)
  WeMosVolts.filter=filterG; // exponential decay digital filter parameter (Mode 1)
  WeMosVolts.filterLevel = filterLevelG ; // No. of filter stages in series (max is 8) (Mode 1)
  WeMosVolts.ignoreThreshold=ignoreThresholdG; // discard ADC readings if (ADC Reading-Average) exceeds this (Modes 1 and 2)
  WeMosVolts.timeAllowed=timeAllowedG; // Mode 2
  WeMosVolts.discardLimit=discardLimitG; // sets Mode 1 discard limit

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
  WeMosVolts.run();
  // check Zero calibration button
  calZero.update(); // update status of Bounce object for Zero button
  zeroButtonRun();
  
  calScale.update(); // update status of Bounce object for Scale button
  scaleButtonRun();

  readVoltageRun();
}


