#ifndef PINS_H
#define PINS_H

#include <pcf8574_esp.h>
#include <Wire.h>

/*
#define NODEMCU_PIN_A0 17
#define NODEMCU_PIN_D0 16	Buzzer
#define NODEMCU_PIN_D1 5	 [I/O]SCL // I2C
#define NODEMCU_PIN_D2 4	 [I/O]SDA // I2C
#define NODEMCU_PIN_D3 0
#define NODEMCU_PIN_D4 2
#define NODEMCU_PIN_D5 14    Pump
#define NODEMCU_PIN_D6 12    Sensor
#define NODEMCU_PIN_D7 13    Heater
#define NODEMCU_PIN_D8 15    AuxHeater
*/

#define BUTTON_USE_EXT false
#define BUTTON_USE_AVR true

#define HEATER_USE_EXT false
#define HEATER_USE_AVR true
#define PUMP_USE_EXT false
#define PUMP_USE_AVR true
#define BUZZER_USE_EXT false
#define BUZZER_USE_AVR true

//#define PUMP_INVERTED_LOGIC true
// with all address pins grounded, PCF8574 is 0x20 while pCF8574A is 0x38
#define PCF8574_ADDRESS 0x20
//#define PCF8574_ADDRESS 0x38

#define AVR_ADDRESS 0x09

//Using ESP8266 PINs as input seems to be a good idea
//#define I2C_SCL NODEMCU_PIN_D1
//#define I2C_SDA NODEMCU_PIN_D2
#define SensorPin    NODEMCU_PIN_D6

#define AuxHeatControlPin NODEMCU_PIN_D8

// the following pin are group into Output & Input
// Input is for button, while output is for heater,pump, and buzzer.

#if BUTTON_USE_EXT == true
#define ButtonUpPin    2 // P1
#define ButtonDownPin  1 // p0 NODEMCU_PIN_D4
#define ButtonStartPin  8 //P3 NODEMCU_PIN_D5
#define ButtonEnterPin  4 //P2 NODEMCU_PIN_D6

#elif BUTTON_USE_AVR == true
#define ButtonCMD       1 // Command to read buttons
#define ButtonUpPin     2 // P1
#define ButtonDownPin   1 // p0 NODEMCU_PIN_D4
#define ButtonStartPin  8 //P3 NODEMCU_PIN_D5
#define ButtonEnterPin  4 //P2 NODEMCU_PIN_D6

#else
#define ButtonUpPin    NODEMCU_PIN_D3
#define ButtonDownPin   NODEMCU_PIN_D4
#define ButtonStartPin  NODEMCU_PIN_D5
#define ButtonEnterPin  NODEMCU_PIN_D7

#endif

#if PUMP_USE_EXT == true
#define ExPumpControlPin  5
#elif PUMP_USE_AVR == true
#define PumpCMD       2 // Command for pump output
#else
#define PumpControlPin  NODEMCU_PIN_D5
#endif

#if HEATER_USE_EXT == true
#define ExHeatControlPin  7
#elif HEATER_USE_AVR == true
#define HeaterCMD       3 // Command to enable Heater output
#define ACRelayCMD       6 // Command to enable AC Relay output
#define PWMCMD       4 // Command to update PWM output
#else
#define HeatControlPin  NODEMCU_PIN_D7
#endif

#if BUZZER_USE_EXT == true
#define ExBuzzControlPin 6
#elif BUZZER_USE_AVR == true
#define BuzzCMD       5 // Command for Buzzer output
#else
#define BuzzControlPin NODEMCU_PIN_D0
#endif


#if (BUTTON_USE_EXT == true) || (HEATER_USE_EXT == true) || (PUMP_USE_EXT == true) ||( BUZZER_USE_EXT == true)
PCF8574 pcf8574(PCF8574_ADDRESS,I2C_SDA, I2C_SCL);
#endif

// Buttons are INPUT
// byte btnReadPin(byte p){ return digitalRead(p);}

#if BUTTON_USE_EXT == true
  byte _portvalue;
  void btnPrepareRead(void) {
	_portvalue=pcf8574.read8();
  }

  byte btnReadPin(byte pin){
	return (_portvalue & pin);
  }

#elif BUTTON_USE_AVR == true
  byte _portvalue;
  void btnPrepareRead(void) {
    Wire.beginTransmission(AVR_ADDRESS);
    Wire.write(ButtonCMD); // Transfer command ("1") to get button value;
    Wire.write(0); // Transfer command ("1") to get button value;
    delay(100);
    // GET RESPONSE
    Wire.requestFrom(AVR_ADDRESS, 1);
    _portvalue = Wire.read();
    int error = Wire.endTransmission();
	//if (error != 0) {_portvalue = 255;}
  }

  byte btnReadPin(byte pin){
	return (_portvalue & pin);
  }

#else

void btnPrepareRead(void){}
#define btnReadPin digitalRead

#endif

byte oldpwm = 0;
inline void setHeaterPWM(byte v) 
#if HEATER_USE_AVR == true
{
    if (oldpwm != v) {
      Wire.beginTransmission(AVR_ADDRESS);
      Wire.write(PWMCMD); // Transfer command ("8") to set pin command
      Wire.write(v); // Transfer command ("8") to set pin command
      int error = Wire.endTransmission();
      oldpwm = v;
    }
}
#else
{}
#endif

inline void setHeaterRelay(byte v) 
#if HEATER_USE_AVR == true
{
      Wire.beginTransmission(AVR_ADDRESS);
      Wire.write(ACRelayCMD); // Transfer command ("8") to set pin command
      Wire.write(v); // Transfer command ("8") to set pin command
      int error = Wire.endTransmission();
}
#else
{}
#endif
// Heater, Pump, Buzz are OUTPUTs
inline void setHeaterOut(byte v)
{
#if HEATER_USE_EXT == true
	pcf8574.write(ExHeatControlPin,v);
#elif HEATER_USE_AVR == true
    Wire.beginTransmission(AVR_ADDRESS);
    Wire.write(HeaterCMD); // Transfer command ("8") to set pin command
    Wire.write(v); // Transfer command ("8") to set pin command
    int error = Wire.endTransmission();
    oldpwm = 0;
#else
	digitalWrite (HeatControlPin, v);
#endif
}

inline void setPumpOut(byte v)
{
#if PUMP_USE_EXT == true
	pcf8574.write(ExPumpControlPin,v);
#elif PUMP_USE_AVR == true
    Wire.beginTransmission(AVR_ADDRESS);
    Wire.write(PumpCMD); // Transfer command ("8") to set pin command
    Wire.write(v); // Transfer command ("8") to set pin command
    int error = Wire.endTransmission();
#else

#if PUMP_INVERTED_LOGIC
	digitalWrite (PumpControlPin, (v==LOW)? HIGH:LOW);
#else
	digitalWrite (PumpControlPin, v);
#endif

#endif
}

inline void setBuzzOut(byte v)
{
#if BUZZER_USE_EXT == true
	pcf8574.write(ExBuzzControlPin,v);
#elif BUZZER_USE_AVR == true
    Wire.beginTransmission(AVR_ADDRESS);
    Wire.write(BuzzCMD); // Transfer command ("8") to set pin command
    Wire.write(v); // Transfer command ("8") to set pin command
    int error = Wire.endTransmission();
#else
	digitalWrite (BuzzControlPin, v);
#endif
}

#if SpargeHeaterSupport == true
inline void setAuxHeaterOut(byte v)
{
	digitalWrite (AuxHeatControlPin, v);
}
#endif

#if SecondaryHeaterSupport == true
inline void setSecondaryHeaterOut(byte v)
{
	digitalWrite (AuxHeatControlPin, v);
}
#endif

void initIOPins(void)
{


#if (BUTTON_USE_EXT == true) || (HEATER_USE_EXT == true) || (PUMP_USE_EXT == true) ||( BUZZER_USE_EXT == true)
//	pcf8574.begin();
#endif

#if (BUTTON_USE_AVR == true) || (HEATER_USE_AVR == true) || (PUMP_USE_AVR == true) ||( BUZZER_USE_AVR == true)
    //Wire.begin();       // join i2c bus sda,scl

#endif

#if BUTTON_USE_EXT == true
	_portvalue=0;

#elif BUTTON_USE_AVR == true
    _portvalue=0;

#else
  	pinMode (ButtonUpPin,    INPUT_PULLUP);
  	pinMode (ButtonDownPin,    INPUT_PULLUP);
  	pinMode (ButtonStartPin, INPUT_PULLUP);
  	pinMode (ButtonEnterPin, INPUT_PULLUP);
#endif

#if (HEATER_USE_EXT == false) && (HEATER_USE_AVR == false)
	pinMode (HeatControlPin, OUTPUT);
#endif
	setHeaterOut(LOW);

#if (PUMP_USE_EXT == false) && (PUMP_USE_AVR == false) 
	pinMode (PumpControlPin, OUTPUT);
#endif
	setPumpOut(LOW);

#if (BUZZER_USE_EXT == false) && (BUZZER_USE_AVR == false) 
	pinMode (BuzzControlPin, OUTPUT);
#endif
	setBuzzOut(LOW);

#if SpargeHeaterSupport == true
	pinMode (AuxHeatControlPin, OUTPUT);
	setAuxHeaterOut(LOW);
#endif

}
#endif
