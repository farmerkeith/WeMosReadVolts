// tab file for LED object
// created 10 Oct 2017 by farmerkeith
// some of this code has been copied from the standard LED.h file

// include guard
#ifndef LED_H
#define LED_H


class LED{
  public:
  // constructor
    LED(uint8_t ledPin, bool Vbehind);
  // functions
  bool getState(); // return HIGH for ON, LOW for OFF
  void on();       // turns on the LED
  void off();      // turns off the LED
  void toggle();   // turns on if off, and off if on
  void run();      // to be run often (eg every loop())
                   // controls blink and fade timing
//  void blink(unsigned int period, byte flashes); // blinks the LED
    // with a period of "period", 50% duty cycle, "flashes" times
    // standard implementation uses delay() 
//  void setValue(unsigned int val); // brightness between ON and OFF using PWM
    // fully ON is val=1023, fully OFF is val=0
//  void fadeIn(unsigned int time); // increases from 0 to 100% over "time"
//  void fadeOut(unsigned int time);// decreases from 100% to 0 over "time"
  private:
  // variables
  bool status; // HIGH (1) for ON, LOW (0) for OFF
  uint8_t pin;
  bool _Vbehind; // 1 if LED has Vcc behind it; 0 if GND is behind it
  unsigned long  ledTime; // keep track of time
  byte period=1; // ms, interval between executions in LED::run()
  public:
//  unsigned int _val; // added for debugging
};

// constructor
LED::LED(uint8_t ledPin, bool Vbehind){
  pin=ledPin;
  _Vbehind = Vbehind;
  pinMode(pin,OUTPUT);
  off();
}

void LED::run(){
  if((long)(millis() - ledTime) >= 0){ // time is expired
    ledTime += period; // ms, set time for the following execution
    
  }
}

bool LED::getState() {return status; }

void LED::on(void){
  analogWrite(pin,0);
  if (_Vbehind) {
    digitalWrite(pin,LOW);
  } else {
    digitalWrite(pin,HIGH);
  }
  status=true;
}

void LED::off(void){
  analogWrite(pin,0);
  if (_Vbehind) {
    digitalWrite(pin,HIGH);
  } else {
    digitalWrite(pin,LOW);
  }
  status=false;
}
  
void LED::toggle(void){
  if (status) off(); else on();
//  status ? off() : on();
}

/*
void LED::blink(unsigned int period, byte flashes){
  for (byte i=0; i<flashes; i++){
    toggle();
    delay(period/2);
    toggle();
    delay(period/2);
  }
}

// uses PWM (any ESP8266 GPIO)
void LED::setValue(unsigned int val){
  if (val>1008) val=1008; // PWM range is 0 to 1023, 1008 is 1024-16
  if (_Vbehind) { // brightness increases with decreasing PWM duty 
    analogWrite(pin,1008-val);
  } else { // brightness increases with increasing PWM duty 
    analogWrite(pin,val);
  }
  status = val>>9; // 1 if val >= 512
  
  Serial.print("setValue val(G)=");
  Serial.print(val);
  Serial.print (" 1008-val(V)=");
  Serial.println (1008-val);
  _val = val;

//  val>=511 ? this->status=false : this->status=true;
}

// uses PWM (any ESP8266 GPIO)
void LED::fadeIn(unsigned int time){
  for (int value = 0 ; value <1024; value+=16){ 
    setValue (value);
 //    analogWrite(pin, value);
    delay(time/(64)); // 64 = 1024 / 16
    // breaks if time < 64!
  } 
 //  analogWrite(pin,0);
  on();
}

//assume PWM
void LED::fadeOut(unsigned int time){
  for (int value = 0; value <1023; value+=16){ 
    analogWrite(pin, value); 
    delay(time/(1024/16)); 
  }
  analogWrite(pin, 0); 
  off();
}
*/

#endif

