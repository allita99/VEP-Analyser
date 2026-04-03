#include "vepSOR.h"


int calibrateSensor(int& pwm, int& input, int& dCycle, int& sensorStatus) {
    int adjustmentValue = 128 / 2; // added or subtracted from duty cycle
    unsigned long loopDuration = 1500; // 1.5-second main loop
    unsigned int changeCounter = 0;
    unsigned int scanInterval = 1;
    float peakCounter = 0;
    dCycle = 0b10000000; // Middle value
    bool ableToDetectChanges = false;
    unsigned long currentTime;
    unsigned long prevTime = 0;
    sensorStatus = 0;
    dutyCycleCounter = 0;
    ledcWrite(pwm, dCycle);
    int pinValue = digitalRead(input);
    int prevPinValue = pinValue;

    while (!ableToDetectChanges) {
        unsigned long delayStartTime = millis();
        while (millis() - delayStartTime < 150) {
            // elevator music (wait for capacitor to charge)
        }
        startTime = millis();
        while (millis() - startTime < loopDuration) {
            currentTime = millis();
            if (currentTime - prevTime >= scanInterval) {
                prevTime = currentTime;
                pinValue = digitalRead(input);
                if (pinValue) {peakCounter++;} else {peakCounter--;}

                if (pinValue != prevPinValue) {
                    changeCounter++;
                    prevPinValue = pinValue;
                }
            }
        }

        if (changeCounter > 5) {
            ableToDetectChanges = true;
            Serial.println("Calibration complete.");
            Serial.println(dCycle);
            sensorStatus = 1; 
            calibrateCommandReceived = false;
            return dCycle;
        } else {
            if (peakCounter >= 5) {
                dCycle += adjustmentValue;
                adjustmentValue /= 2;
                Serial.print(">: ");
                Serial.println(ledcRead(pwm));
                sensorStatus = 0;
            } else if (peakCounter <= -5) {
                dCycle -= adjustmentValue;
                adjustmentValue /= 2;
                Serial.print("<: ");
                Serial.println(ledcRead(pwm));
                sensorStatus = 0;
            }

            if (dutyCycleCounter >= 3) {
                calibrateCommandReceived = false;
                Serial.println("Calibration failed.");
                sensorStatus = 0;
                return dCycle;
            }

            changeCounter = 0;
            ledcWrite(pwm, dCycle);
            if (previousDutyCycle == dCycle) {
                dutyCycleCounter++;
            }
            previousDutyCycle = dCycle;
        }
    }

    sensorStatus = 0;
    calibrateCommandReceived = false;
    return dCycle;
}


int calibrate() { 
  // INITIAL PARAMETERS
  int adjustmentValue = 128 / 2; // added or substracted from d_cycle
  unsigned long loopDuration = 1500; // 1.5 second main loops
  unsigned int changeCounter = 0;
  unsigned int scanInterval = 1;
  float peakCounter = 0;
  fixedDutyCycle = 0b10000000; // Set threshold to middle value (128 or half the reference voltage) 
  bool ableToDetectChanges = false;
  unsigned long currentTime;
  unsigned long prevTime = 0; 

  dutyCycleCounter = 0;
  ledcWrite(pwmPin, fixedDutyCycle);
  int pinValue = digitalRead(inputPin);
  int prevPinValue = digitalRead(inputPin);

  while (!ableToDetectChanges) {
    unsigned long delayStartTime = millis();
    Serial.println("wait 300");
    while (millis() - delayStartTime < 300) {
        //Elevator music
      }
    Serial.println("start timer");
    startTime = millis(); // Reset timer
    Serial.print("wait ");
    Serial.println(loopDuration);
    while (millis() - startTime < loopDuration) { // Test the sensor for 1.5 seconds
      currentTime = millis();
      if (currentTime - prevTime >= scanInterval) { // Read Digital input once every 1ms
        prevTime = currentTime;

        pinValue = digitalRead(inputPin);
        if (pinValue) {
          peakCounter++;
        } else {
          peakCounter--;
        }
        if (pinValue != prevPinValue) {
          changeCounter++; // Count changes
          prevPinValue = pinValue; // Update the previous value
        }
      }
    }

    if (changeCounter > 5) { // more than 4 changes detected
      ableToDetectChanges = true;
      Serial.println("Calibration complete.");
      Serial.println(fixedDutyCycle);
      calibrateCommandReceived = false;
      return fixedDutyCycle;
    } else {
      if (peakCounter >= 5) {  
        fixedDutyCycle = fixedDutyCycle + adjustmentValue; // Increase by current adjustment
        adjustmentValue = adjustmentValue / 2; // Halve the adjustment for next iteration
        peakCounter = 0;
        Serial.print(">: ");
        Serial.println(ledcRead(pwmPin));
    } else if (peakCounter <= -5) {
        fixedDutyCycle = fixedDutyCycle - adjustmentValue; // Decrease by current adjustment
        adjustmentValue = adjustmentValue / 2; // Halve the adjustment for next iteration
        peakCounter = 0;
        Serial.print("<: ");
        Serial.println(ledcRead(pwmPin));
    } 
      if(dutyCycleCounter >= 3){
        calibrateCommandReceived = false;
        
        Serial.println("Calibration failed.");
        Serial.println(ledcRead(pwmPin));
        return fixedDutyCycle;
      }
      changeCounter = 0; // Reset Counter
      ledcWrite(pwmPin, fixedDutyCycle);
      if(previousDutyCycle == fixedDutyCycle){
          dutyCycleCounter++;
      }
      previousDutyCycle = fixedDutyCycle;
    }
    
  }
  calibrateCommandReceived = false;
  return fixedDutyCycle;
}

void enableSensorPower(int& sensorControlPin){
   digitalWrite(sensorControlPin, HIGH);
   //return 0;
}
void disableSensorPower(int& sensorControlPin){
  digitalWrite(sensorControlPin, LOW);
  //return 0;
}

int testSensorConnection(int& pwm, int& input, int& dCycle, int& sensorStatus) {
    unsigned int changeCounter = 0;
    unsigned int originalDutyCycle = dCycle;
    unsigned long delayStartTime;

    // sensorStatus = 1;
    // return sensorStatus;
    //dCycle = 0; // Start with duty cycle set to 0
    sensorStatus = 0; // Initialize sensor status
    
    // Test with duty cycle set to 0
   // Serial.println("Sensor low state test.");
    ledcWrite(pwm, 0);

    delayStartTime = millis();
    while (millis() - delayStartTime < 500) {
        //Elevator music
      }

    int pinValue = digitalRead(input);

    if (pinValue == 1) {
          Serial.print("Sensor high state test: passed ||");
          //Serial.println("Sensor high state test.");
          ledcWrite(pwm, 255); // set 100% duty cycle
          delayStartTime = millis();
          while (millis() - delayStartTime < 500) {
              //Elevator music
            }
          pinValue = digitalRead(input);

          if (pinValue == 0) {
            Serial.print("Sensor low state test: passed");
            sensorStatus = 1; // Set sensor status to indicate success
          } else {
            sensorStatus = 0; 
            Serial.print("Sensor low state test: failed");
          }
    } else {
        sensorStatus = 0;
        Serial.print("Sensor high state test: failed");
    }

    

    // Reset duty cycle to a default state (e.g., 0)
    
    if(sensorStatus == 0)
  	  dCycle = 0;
      
    ledcWrite(pwm, dCycle);
    delayStartTime = millis();
    while (millis() - delayStartTime < 500) {
        //Elevator music
      }
    Serial.println("");
    return sensorStatus;
}

void enableTestingLED(int& pwm){
  ledcWrite(pwm, 128);
  // int delayStartTime = millis();
  //   while (millis() - delayStartTime < 50) {
  //       //Elevator music
  //     }
  //return 0;
}
void disableTestingLED(int& pwm){
  ledcWrite(pwm, 0);
  // int delayStartTime = millis();
  //   while (millis() - delayStartTime < 50) {
  //       //Elevator music
  //     }
  //return 0;
}


// int calibrateSensor1(int& sensorCheck) {
//   if(digitalRead(sensorCheck))    {return calibrateSensor(pwmPin, inputPin, fixedDutyCycle, 1);}
//   else{ Serial.println(F("Sensor 1 not detected.")); return 0; }
// }

// int calibrateSensor2(int& sensorCheck) {
//   if(digitalRead(sensorCheck))    {return calibrateSensor(pwmPin2, inputPin2, fixedDutyCycle2, 1);}
//   else{ Serial.println(F("Sensor 2 not detected."));  return 0; }
// }