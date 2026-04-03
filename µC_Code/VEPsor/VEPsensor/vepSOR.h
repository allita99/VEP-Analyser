#ifndef VEPSOR_H
#define VEPSOR_H

#include <Arduino.h>

// Declare global or static variables



extern int pwmPin;
extern int inputPin;

extern int fixedDutyCycle;
extern int dutyCycleCounter;
extern int previousDutyCycle;

extern int pwmPin2; // Sensor TWO PWM pin
extern int inputPin2; // Sensor TWO signal pin

extern int fixedDutyCycle2; // Fixed duty cycle value (0-255)
extern int previousDutyCycle2;
extern int dutyCycleCounter2; 

extern unsigned long startTime;
extern bool calibrateCommandReceived;
extern int previousDutyCycle; // If needed


int calibrate(); // old calibrate
// int calibrateSensor1(int& sensorCheck); // quick calibrate call for each sensor
// int calibrateSensor2(int& sensorCheck); // quick calibrate call for each sensor
void enableSensorPower(int& sensorControlPin);
void disableSensorPower(int& sensorControlPin);
int calibrateSensor(int& pwm, int& input, int& dCycle, int& sensorStatus); //new calibrate
int calibrateSensor(int& pwm, int& input, int& dCycle, int& sensorStatus); //new calibrate
int testSensorConnection(int& pwm, int& input, int& dCycle, int& sensorStatus);
void enableTestingLED(int& pwm);
void disableTestingLED(int& pwm);

#endif
