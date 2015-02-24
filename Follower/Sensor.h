#include <stdint.h>
#include <Arduino.h>

#ifndef SENSOR_H
#define SENSOR_H

#define numCalibs 100
#define unattached -1

#ifdef ARDUINO_UNO
	#define upperAnalogPin 5
	#define upperDigitalPin 13
#else 	
	#define upperAnalogPin 15
	#define upperDigitalPin 53
#endif

#define validAnalogPin(pin) ((pin) >= 0 && (pin) <= upperAnalogPin)
#define validDigitalPin(pin) ((pin) >= 0 && (pin) <= upperDigitalPin)
 	
class Sensor {
private:
	uint16_t threshold;		
	uint16_t val;

	uint8_t pin;
	uint8_t emitterPin;

public:
	Sensor();
	Sensor(uint8_t);
	Sensor(uint8_t, uint8_t);

	uint8_t attach(uint8_t);	
	uint8_t attach(uint8_t, uint8_t);
	uint8_t detach();

	uint8_t calibrate();
	uint16_t read();

	uint8_t getBit();
	uint16_t getThreshold();
	uint16_t getVal();
};

Sensor::Sensor() {
	this->threshold = 0;
	this->val = 0;
	this->pin = unattached;
	this->emitterPin = unattached;
}

Sensor::Sensor(uint8_t pin) {
	this->threshold = 0;
	this->val = 0;
	this->attach(pin);
	this->emitterPin = unattached;
}

Sensor::Sensor(uint8_t pin, uint8_t emitterPin) {
	this->threshold = 0;
	this->val = 0;
	this->attach(pin, emitterPin);
}


/*	Precondition: "pin" is within Arduino's valid range.
	Postcondition: Sensor's pin is assigned parameter, returns 0.
	Error: Sensor pin is set to "unattached", returns 1. */

uint8_t Sensor::attach(uint8_t  pin) {
	if(!validAnalogPin(pin)) {
		this->pin = unattached;
		return 1;
	}

	this->pin = pin;	
	return 0;	
}

/*	Precondition: "pin" and "emitterPin" is within Arduino's valid range.
	Postcondition: Sensor's pin and emitter are assigned, return 0.
	Error: Sensor's pin and emitter are set to "unattached", returns 1.*/

uint8_t Sensor::attach(uint8_t pin, uint8_t emitterPin) {
	if(!validAnalogPin(pin) || !validDigitalPin(emitterPin)) {
		this->pin = unattached;
		this->emitterPin = unattached;
		return 1;
	}

	this->pin = pin;
	this->emitterPin = emitterPin;	
	return 0;
}

uint8_t Sensor::detach() {
	this->pin = unattached;
	this->emitterPin = unattached;
}

/*	Precondition: Sensor's pins are assigned.
	Postcondition: Sensor's threshold value is calculated
	by reading "numCalibs" number of readings and finding
	the average of max and min.
	Error: No error checks are made; however, Sensor's threshold
	may need to be offset by a bias during actual testing. */

uint8_t Sensor::calibrate() {

	uint16_t max, min; 
	uint16_t vals[numCalibs] = {};
        
	for(uint16_t i = 0; i < numCalibs; i++) {
		vals[i] = this->read();
		if(vals[i] > max || i < 1)
			max = vals[i];
	 	if(vals[i] < min || i < 1)
     	min = vals[i];
	}
        
	this->threshold = (max + min) / 2;        
   
	return 0;
}

/*	Precondition: Sensor's pin and emitter are assigned.
	Postcondition: Sensor takes its pin's an analog reading when
	the emitter is on another when it's off. This is done to reduce
	the effect of ambient light on the IR sensor readings.
	Error: If Sensor's pin is unattached, returns 0. 
	If emitter pin is unattached, a single Sensor reading is returned. */

uint16_t Sensor::read() {
	
	if(this->pin == unattached)
		return 0;

  if(this->emitterPin == unattached) 
		this->val = analogRead(this->pin);
	else {
		uint16_t ambientVal = 0;
	
		digitalWrite(this->emitterPin, HIGH);	
		this->val = analogRead(this->pin); // take reading with emitters on

		delayMicroseconds(250); // switching too fast causes jitters
	
		digitalWrite(this->emitterPin, LOW);	
		ambientVal = analogRead(this->pin); // take another with emitters off

		this->val -= ambientVal; // adjust reading for ambient reflections
	}

	return this->val;
}

/*	Precondition: Sensor has been calibrated, and reading made.
	Postcondition: If Sensor's reading is above threshold, returns 1.
	Returns 0 otherwise. */

uint8_t Sensor::getBit() {
	if(this->val > this->threshold)
		return 1;
	return 0;
}

uint16_t Sensor::getVal() {
	return this->val;
}

uint16_t Sensor::getThreshold() {
	return this->val;
}


/*	Precondition: Sensors in "sensors" array have been calibrated.
	Postcondition: Writes binary state of the nth sensor in "sensor" array
	to the nth bit of "sensorBinary" and returns it.
*/

uint8_t readSensorsBinary(Sensor* sensors, uint8_t numSensors) {
	uint8_t sensorBinary = B00000000;
	for(uint8_t i = 0; i < numSensors; i++) 
		bitWrite(sensorBinary, i, sensors[i].getBit());

	return sensorBinary;
}

uint16_t readSensorsAnalog(Sensor* sensors, uint8_t numSensors) {
	uint16_t sensorAnalog = 0;
	for(uint8_t i = 0; i < numSensors; i++)
		sensorAnalog += sensors[i].read();

	return sensorAnalog;
}

#endif
