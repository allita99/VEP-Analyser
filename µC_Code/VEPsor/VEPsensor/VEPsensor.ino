#include <Arduino.h>
#include "dataSampling.h"
#include "displayManager.h"
#include "vepSOR.h"
#include <esp_sleep.h>

#include <EEPROM.h>
#define EEPROM_SIZE 6 // define the number of bytes you want to access

#define SDA_1 21
#define SCL_1 22

//Pins for light detector
// SENSOR ONE
const int pwmChannel = 0;int pwmPin = 2;int inputPin = 4;int sensorCheck1 = 0;
int fixedDutyCycle = 124;int previousDutyCycle = 124;int dutyCycleCounter = 0;
int enSens = 25;
// SENSOR TWO
const int pwmChannel2 = 1;int pwmPin2 = 5;int inputPin2 = 18;int sensorCheck2 = 0;
int fixedDutyCycle2 = 124;int previousDutyCycle2 = 124;int dutyCycleCounter2 = 0;
int enSens2 = 26;
// SENSOR THREE
const int pwmChannel4 = 3; int pwmPin4 = 27; int inputPin3 = 19;
int fixedDutyCycle3 = 124;int previousDutyCycle3 = 124;int dutyCycleCounter3 = 0;

//LED OnBoard Stimuli
const int pwmChannel3 = 2;int pwmPin3 = 15;bool onBoardStimulusEnabled = false;
//Buttons
const int button1 = 13;const int button2 = 14;
const int batteryStatusCheck = 35;


bool startCommandReceived = false;bool calibrateCommandReceived = false;
bool startSavingCommandReveived = false;bool scanModecVEP = false; bool scanModeSSVEP = false;
unsigned long startTime = 0;
unsigned long previousMicros = 0;


// FOR RECORDING AND MANAGING SAVED DATA
const int duration = 10;      //seconds
const int sampleRate = 1000;  //1Khz
const int totalSamples = sampleRate * duration;
const int byteSize = 313;   //1,250 bytes for 10,000 samples
const int byteSize2 = 180;  //old 720  //1,250 bytes for 10,000 samples
const int bitSize = 10016;  //10000;
const int bitSize2 = 5760;  //5760;
uint32_t sensorData1[313];  //array that will keep the data from sensor1
uint32_t sensorData2[313];  //array that will keep the data from sensor2 might make it conditional (only declare if second sensor is connected in case of lack of memory)
uint32_t reData[180];
uint32_t reData2[180];
uint32_t memoryData[50];


uint32_t debounceTime = 400;  //250ms debounce protection
uint32_t lastButton1Press = 0;
uint32_t lastButton2Press = 0;
int previousButton1State = 0;
int previousButton2State = 0;

int currentSample = 0;  //current location
//bool dualSense = false;          // flag for dual sensor connection
//bool sensorSetupChange = false;
bool forceStop = false;
int pinValue = 0;   //
int pinValue2 = 0;  //
char buf[20];



const int freq = 5000;     // PWM Frequency in Hz
const int resolution = 8;  // Resolution in bits

int communicationSpeed = 1000;
uint32_t idleCounter = 0;
unsigned long lastIncrementTime = 0; 
unsigned long currentTime = millis();
const uint32_t idleThreshold = 120000;
bool isDeviceIdle = true;
int currentMode = 0;
int previousMode = 0;
bool isRecording = false;
String batteryStatus = "";
bool initialStart = true;

static double measuredFrequency = 0.0;
int recordedAmount = 0;
double freqArray[10] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
int freqArrayIndex = 0;
bool enableMemReload = true;
bool disableSensorCheck = true;
int totalModes = 5;

void wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

  if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
    Serial.println("");
      Serial.println("Wakeup caused by timer. Reseting memory!");
      EEPROM.write(0, 0);EEPROM.write(1, 0);EEPROM.write(3, 0);
      EEPROM.write(4, 0);EEPROM.write(5, 0);EEPROM.commit();
      delay(10);
      esp_deep_sleep_start();}
  else{
    esp_sleep_enable_timer_wakeup(3600000000); // 1 hour = 3600 000 000 us  3600000000
  }
}

float standardDeviation(double array[], int recordCount){
  float allAdded = 0;
  float mean = 0;
  if(recordCount < 0){
     return 0.0;
  }
  else if(recordCount > 10){recordCount = 10;}
  //calculte mean
  for(int i = 0; i < recordCount; i++){allAdded = allAdded + array[i];}
  mean = allAdded / recordCount;allAdded = 0;
  //Calculate sigma part
  for(int j = 0; j < recordCount;j++){
      allAdded = allAdded + ((array[j]-mean) * (array[j]-mean));
  }
  return sqrt(allAdded / recordCount);
}

void resetSensorData(){
  for(int i = 0; i < 313; i++){
    sensorData1[i] = 0UL;
    sensorData2[i] = 0UL;
    if(i < 180)
      reData[i] = 0UL;
      reData2[i] = 0UL;
  }
}

float measureBattery() {
    const float upperVoltageLimit = 3.7;  // Fully charged voltage
    const float lowerVoltageLimit = 3.2;  // Empty voltage
    const float voltageDividerFactor = 2.0;  // Voltage divider factor
    const int adcResolution = 4095;  // ADC resolution for 12 bits
    const float V_ref = 3.3;  // Reference voltage for ADC

    int rawValue = analogRead(batteryStatusCheck);  // Read ADC value
    float V_measured = rawValue * (V_ref / adcResolution);  // Convert ADC value to measured voltage
    float V_battery = V_measured * voltageDividerFactor;  // Account for voltage divider

    if (V_battery <= lowerVoltageLimit) {
        return 0.0;  // Battery is empty
    } else if (V_battery >= upperVoltageLimit) {
        return 100.0;  // Battery is fully charged
    } else {
        return ((V_battery - lowerVoltageLimit) / (upperVoltageLimit - lowerVoltageLimit)) * 100.0;
    }
}

void setup() {

  Serial.begin(1000000);  //115200); 250000
  while (!Serial) { delay(10); }
  EEPROM.begin(EEPROM_SIZE);
  //Sensor 1 Setup
  ledcAttachChannel(pwmPin, freq, resolution, pwmChannel);
  pinMode(inputPin, INPUT_PULLDOWN);pinMode(enSens, INPUT_PULLDOWN);
  digitalWrite(enSens, LOW);
  //Sensor 2 Setup
  ledcAttachChannel(pwmPin2, freq, resolution, pwmChannel2); 
  pinMode(inputPin2, INPUT_PULLDOWN);pinMode(enSens2, INPUT_PULLDOWN);
  digitalWrite(enSens2, LOW);

  // Internal Stimuli at 20 Hz
  ledcAttachChannel(pwmPin3, 20, resolution, pwmChannel3); 

 
  //Button Setup, Button1 change mode button 2 toggle task
  pinMode(button1, INPUT_PULLUP);pinMode(button2, INPUT_PULLUP);

  wakeup_reason();//Checks wakup reason and sets timer or deep sleep mode

  // Retrieve previous states from memory 
  if(enableMemReload)
  {
  sensorCheck1 = EEPROM.read(2);
  sensorCheck2 = EEPROM.read(3);
  currentMode = EEPROM.read(4);

  if(sensorCheck1)
  {enableSensorPower(enSens);
  enableSensorPower(enSens2);
  ledcWrite(pwmPin, EEPROM.read(0));
  ledcWrite(pwmPin2, EEPROM.read(1)); //remember previous duty cycles  EEPROM.write(address, value);
  }
  if(sensorCheck2)
  {enableSensorPower(enSens);
  enableSensorPower(enSens2);
  ledcWrite(pwmPin, EEPROM.read(0));
  ledcWrite(pwmPin2, EEPROM.read(1));}
  }
  disableSensorPower(enSens);
  disableSensorPower(enSens2);

  initScreen();
  //bool ledcAttachChannel(uint8_t pin, uint32_t freq, uint8_t resolution, uint8_t channel);
  analogReadResolution(12);
  //resetDisplay();
  
  resetSensorData();
  //displayTwoLinesOfText(F("Main Menu"), batteryStatus);
}

void checkSerialCommands() {

  String command = Serial.readStringUntil('\n');
  if (command == "START" || command == "start") {
    currentSample = 0;  //reset location
    isDeviceIdle = false;
    startCommandReceived = true;
    calibrateCommandReceived = false;
    startSavingCommandReveived = false;
    isRecording = true;
    enableSensorPower(enSens);
    enableSensorPower(enSens2);
    //resetSensorData();
    // Serial.print(F("Duty cycle set to:"));
    // Serial.println(fixedDutyCycle);
    startTime = micros();  // Reset timer
    Serial.println(F("Start command received. Beginning data acquisition."));
    displayTwoLinesOfText(F("Beginned data acquisition!"), F(""));
    //ledcWrite(pwmPin, fixedDutyCycle);
    //delay(500);
  } else if (command == "STOP" || command == "stop" || forceStop) {
    startCommandReceived = false;
    calibrateCommandReceived = false;
    startSavingCommandReveived = false;
    disableSensorPower(enSens);
    disableSensorPower(enSens2);
    isRecording = false;
    forceStop = false;
    scanModecVEP = false;
    scanModeSSVEP = false;
    Serial.println(F("Stop command received. Halting data acquisition."));
    displayTwoLinesOfText(F("Halted data acquisition!"), F(""));

    unsigned long startTime = millis();  // Record start time
    uint8_t tapPoint = 0;
    uint8_t detectedPolynomial = 0;
    // Perform the calculations
    for (int i = 0; i < 48; i++) {
      //Serial.print(24+i);
      downSample32(sensorData1, reData, byteSize, 24 + i);  // Downsample data
      tapPoint = compareSequences32(reData, (24 + i) * 10, 30);
      if (tapPoint != 0b00000000)    // Compare sequences
      {
        Serial.println("Sensor1: ");
        Serial.print(24 + i);
        Serial.println("");
        Serial.println(getPolynomial(tapPoint));
        if((detectedPolynomial != tapPoint) && (detectedPolynomial == 0)){
            detectedPolynomial = tapPoint;
        }
        if((detectedPolynomial == tapPoint) && (detectedPolynomial != 0)){
           detectedPolynomial = tapPoint;
        }
      }
      
    }

    unsigned long elapsedTime = millis() - startTime;  // Calculate elapsed time

    // Print the elapsed time
    Serial.print(F("Calculation Time: "));
    Serial.print(elapsedTime);
    Serial.println(F(" ms"));

    // TEST TODO
    isDeviceIdle = true;
    
  } else if (command == "CALIBRATE" || command == "cal") {
  enableSensorPower(enSens);
    enableSensorPower(enSens2);
    Serial.println(F("Calibration command received."));
        currentMode = 0;
        startCommandReceived = false;
        startSavingCommandReveived = false;
        calibrateCommandReceived = true;
        scanModecVEP = false;
        scanModeSSVEP = false;
        isDeviceIdle = false;
        fixedDutyCycle = 0b10000000;
        fixedDutyCycle2 = 0b10000000;
        ledcWrite(pwmPin, fixedDutyCycle);
        ledcWrite(pwmPin2, fixedDutyCycle2);
        Serial.println(F("Duty Cycle Reset!"));
        isDeviceIdle = true;

  } else if (command == "DUTY" || command == "duty") {
    startCommandReceived = false;
    calibrateCommandReceived = false;
    startSavingCommandReveived = false;
    isDeviceIdle = false;
    Serial.println(F("Enter the duty cycle value (0-255):"));
    unsigned long startTime = millis();
    const unsigned long timeout = 5000;  // 5 seconds timeout

    while (Serial.available() == 0) {
      if (millis() - startTime > timeout) {
        Serial.println(F("Timeout: No input received."));
        return;  // Exit the function if no input is received within the timeout period
      }
      // Elevator Music
    }
    int dCycle = Serial.parseInt();
    fixedDutyCycle = dCycle;
    ledcWrite(pwmPin, dCycle);
    //delay(500);
    Serial.print(F("Duty cycle set to: "));
    Serial.println(ledcRead(pwmPin));
    isDeviceIdle = true;
  } else if (command == "SPEED" || command == "speed") {
    startCommandReceived = false;
    startSavingCommandReveived = false;
    calibrateCommandReceived = false;
    isDeviceIdle = false;
    Serial.println(F("Enter the sampling period (us):"));
    unsigned long startTime = millis();
    const unsigned long timeout = 5000;  // 5 seconds timeout

    while (Serial.available() == 0) {
      if (millis() - startTime > timeout) {
        Serial.println(F("Timeout: No input received."));
        return;  // Exit the function if no input is received within the timeout period
      }
      // Elevator Music
    }

    int speed = Serial.parseInt();
    if (speed >= 500) {
      communicationSpeed = speed;
    } else {
      communicationSpeed = 1000;  //default value
    }
    //delay(500);  // delay bad
    Serial.print(F("Sampling period set to (us): "));
    Serial.println(speed);
    isDeviceIdle = true;
  }
}

void stopAndAnalyseCVEP(){
          Serial.println(F("Stop command received."));
          
          startCommandReceived = false;
          calibrateCommandReceived = false;
          startSavingCommandReveived = false;
          isRecording = false;
          forceStop = false;
          scanModecVEP = false;
          scanModeSSVEP = false;

          unsigned long startTime = millis();  // Record start time
          uint8_t tapPoint = 0;uint8_t tapPoint2 = 0;
          uint8_t detectedPolynomial = 0;uint8_t detectedPolynomial2 = 0;
          // Perform the calculations
          for (int i = 0; i < 48; i++) {
            //Serial.print(24+i);
            if(sensorCheck1 && sensorCheck2){ // all sensors are connected
              downSample32(sensorData1, reData, byteSize, 24 + i);  // Downsample data
              tapPoint = compareSequences32(reData, (24 + i) * 10, 30);
              downSample32(sensorData2, reData2, byteSize, 24 + i);  // Downsample data
              tapPoint2 = compareSequences32(reData2, (24 + i) * 10, 30);
            }
            else if(sensorCheck1 ^ sensorCheck2) { // single sensor connected
              downSample32(sensorData1, reData, byteSize, 24 + i);  // Downsample data
              tapPoint = compareSequences32(reData, (24 + i) * 10, 30);
              tapPoint2 = 0;
            }
            
            if (tapPoint != 0b00000000)    // Compare sequences
            {
              Serial.println("Sensor1: ");Serial.print(24 + i);
              Serial.print(" hz, ");Serial.println(tapPoint,BIN);
              Serial.println(getPolynomial(tapPoint));
              if((detectedPolynomial != tapPoint) && (detectedPolynomial == 0)){
                  detectedPolynomial = tapPoint;
              }
              else if((detectedPolynomial == tapPoint) && (detectedPolynomial != 0)){
                  detectedPolynomial = tapPoint;
              }
            }
            if(tapPoint2 != 0b00000000){
              Serial.println("Sensor2: ");Serial.print(24 + i);
              Serial.print(" hz, ");Serial.println(tapPoint2,BIN);
              Serial.println(getPolynomial(tapPoint2));
              if((detectedPolynomial2 != tapPoint2) && (detectedPolynomial2 == 0)){
                  detectedPolynomial2 = tapPoint2;
              }
              else if((detectedPolynomial2 == tapPoint2) && (detectedPolynomial2 != 0)){
                  detectedPolynomial2 = tapPoint2;
              }

            }
            
          }

          unsigned long elapsedTime = millis() - startTime;  // Calculate elapsed time

          // Print the elapsed time
          Serial.print(F("Calculation Time: "));
          Serial.print(elapsedTime);
          Serial.println(F(" ms"));
          // HANDLE DETECTED POLYNOMIALS
          if(detectedPolynomial != 0 && (sensorCheck1 ^ sensorCheck2)) // single sensor connected and a polynomial detected
          {
            displayTwoLinesOfText(F("Polynomial Detected!"), getPolynomial(detectedPolynomial));
          }
          else if((sensorCheck1 && sensorCheck2) ) // both sensors connected and at least one polynomial detected && ( detectedPolynomial || detectedPolynomial2)
          {
              if(detectedPolynomial != 0 && detectedPolynomial2 == 0 )
               displayTwoLinesOfText("s1:"+  getPolynomial(detectedPolynomial) , "s2: NaN" );
              else if(detectedPolynomial == 0 && detectedPolynomial2 != 0 )
               displayTwoLinesOfText("s1: NaN" , "s2:"+  getPolynomial(detectedPolynomial2) );
              else
               displayTwoLinesOfText("s1:"+  getPolynomial(detectedPolynomial) , "s2:"+  getPolynomial(detectedPolynomial2) );
          }
          else   // nothing detected at all
          {displayTwoLinesOfText(F("No Polynomial Detected!"), F(":("));}
          // TEST TODO
          //resetSensorData();
          isDeviceIdle = true;

}

void checkButtonCommands() {
  // Read current button states
  bool button1State = !digitalRead(button1); // LOW means pressed, invert the logic
  bool button2State = !digitalRead(button2); // LOW means pressed, invert the logic
  
  unsigned long currentMillis = millis();

  if (button1State && button2State && (currentMillis - lastButton1Press > debounceTime) && !(calibrateCommandReceived || startCommandReceived)){
    enableSensorPower(enSens);
    enableSensorPower(enSens2);
    Serial.println(F("Calibration command received."));
        currentMode = 0;
        startCommandReceived = false;
        startSavingCommandReveived = false;
        calibrateCommandReceived = true;
        scanModecVEP = false;
        scanModeSSVEP = false;
        isDeviceIdle = false;
        fixedDutyCycle = 0b10000000;
        fixedDutyCycle2 = 0b10000000;
        ledcWrite(pwmPin, fixedDutyCycle);
        ledcWrite(pwmPin2, fixedDutyCycle2);
        Serial.println(F("Duty Cycle Reset!"));
        isDeviceIdle = true;

  }

  // Button 1 - Mode Switching
  if (button1State && (currentMillis - lastButton1Press > debounceTime) // button pressed and debouncing prevented
  && !(calibrateCommandReceived || startCommandReceived)) {              // and no process going on
    lastButton1Press = currentMillis;  // Update debounce timer
    idleCounter = 0; // reset idle counter
    currentMode = (currentMode + 1) % totalModes;  // Cycle through modes
    EEPROM.write(4, currentMode);
    switch (currentMode) {
      case 0:
        batteryStatus =  "Bat: " + String(measureBattery()) + " %";
        Serial.println(F("Mode: Main Menu"));
        displayTwoLinesOfText(F("Main Menu"), batteryStatus);
        break;
      // case 1:
      //   Serial.println(F("Mode: Calibration"));
      //   displayTwoLinesOfText(F("Calibration Mode"), F(""));
      //   break;
      case 3:
        Serial.println(F("Stream Mode: Start/Stop"));
        displayTwoLinesOfText(F("Start/Stop Mode"),  F("Data Stream"));
        break;
      case 1:
        Serial.println(F("SSVEP Mode: Start/Stop"));
        displayTwoLinesOfText(F("Start/Stop Mode"), F("Find frequency."));
        break;
      case 2:
        Serial.println(F("cVEP Mode: Start/Stop "));
        displayTwoLinesOfText(F("Start/Stop Mode"), F("Find polynomial of\nthe m-sequence."));
        break;
      case 4:
        Serial.println(F("Mode: On board Stimuli"));
        //displayThreeLinesOfText(F("Toggle"), F("On board light stimulus."),"");
        if(onBoardStimulusEnabled)
          displayThreeLinesOfText(F("Toggle"), F("Internal Stimuli"),"On");
        else
          displayThreeLinesOfText(F("Toggle"), F("Internal Stimuli"),"Off");
        break;
        EEPROM.commit();
    }
  }

  // Button 2 - Perform Action in Current Mode
  if (button2State && (currentMillis - lastButton2Press > debounceTime)) {
    lastButton2Press = currentMillis;  // Update debounce timer
    idleCounter = 0; // reset idle counter


    switch (currentMode) {
      case 0:  // Main Menu
        Serial.println(F("Main Menu: No action."));
        displayThreeLinesOfText(F("Reset Button"),F("Button1: Change mode."),  F("Button2: Start/Stop."));
        isDeviceIdle = true;
        break;

      // case 1:  // Calibration Mode
      //   Serial.println(F("Calibration command received."));
      //   displayTwoLinesOfText(F("Calibrating..."), F(""));
      //   startCommandReceived = false;
      //   startSavingCommandReveived = false;
      //   calibrateCommandReceived = true;
      //   scanModecVEP = false;
      //   scanModeSSVEP = false;
      //   isDeviceIdle = false;
      //   fixedDutyCycle = 0b10000000;
      //   fixedDutyCycle2 = 0b10000000;
      //   ledcWrite(pwmPin, fixedDutyCycle);
      //   ledcWrite(pwmPin2, fixedDutyCycle2);
      //   Serial.println(F("Duty Cycle Reset!"));
      //   isDeviceIdle = true;
        
      //   break;
      case 3:  // Start/Stop Mode STREAM MODE
        if (isRecording) {
          Serial.println(F("Stop command received."));
          displayTwoLinesOfText(F("Halted"), F("the data stream!"));
          disableSensorPower(enSens);
          disableSensorPower(enSens2);
          startCommandReceived = false;
          calibrateCommandReceived = false;
          startSavingCommandReveived = false;
          isRecording = false;forceStop = false;
          scanModecVEP = false;scanModeSSVEP = false;

          unsigned long startTime = millis();  // Record start time
          uint8_t tapPoint = 0;
          unsigned long elapsedTime = millis() - startTime;  // Calculate elapsed time

          // Print the elapsed time
          Serial.print(F("Calculation Time: "));
          Serial.print(elapsedTime);
          Serial.println(F(" ms"));

          // TEST TODO
          isDeviceIdle = true;
        } else {
          if(sensorCheck1 || sensorCheck2){
            Serial.println(F("Start command received."));
            displayTwoLinesOfText(F("Streaming data!"), F(""));
            enableSensorPower(enSens);
            enableSensorPower(enSens2);
            isRecording = true;
            currentSample = 0;  //reset location
            isDeviceIdle = false;
            startCommandReceived = true;
            calibrateCommandReceived = false;
            startSavingCommandReveived = false;
            isRecording = true;
            scanModecVEP = false;
            scanModeSSVEP = false;
            startTime = micros();
          }else{
              Serial.println(F("No sensors detecteed."));
              displayTwoLinesOfText(F("No sensor"), F("data detected!!!"));
              isDeviceIdle = true;
            }
        }
        break;
      
      case 1:  // Start/Stop Mode ssvep anaylsis
        if (isRecording) {
          Serial.println(F("Stop command received."));
          //displayTwoLinesOfText(F("Halted frequency"), F("analysis!"));
          //displayTwoLinesOfText("Frequency analysis:", "\nf= " + String(measuredFrequency, 4) + " Hz");
          disableSensorPower(enSens);
          disableSensorPower(enSens2);
          displayThreeLinesOfText("Frequency analysis:", "f= " + String(measuredFrequency, 4) + " Hz", "+-" + String(standardDeviation(freqArray, recordedAmount),4)+ " <");
          startCommandReceived = false;
          calibrateCommandReceived = false;
          startSavingCommandReveived = false;
          isRecording = false;forceStop = false;
          scanModecVEP = false;scanModeSSVEP = false;
          recordedAmount = 0;
          freqArrayIndex = 0;
          unsigned long startTime = millis();  // Record start time
          uint8_t tapPoint = 0;
          unsigned long elapsedTime = millis() - startTime;  // Calculate elapsed time

          // Print the elapsed time
          Serial.print(F("Calculation Time: "));
          Serial.print(elapsedTime);
          Serial.println(F(" ms"));

          // TEST TODO
          isDeviceIdle = true;
        } else {
          if(sensorCheck1 || sensorCheck2){
            Serial.println(F("Start command received."));
            enableSensorPower(enSens);
            enableSensorPower(enSens2);
            //displayThreeLinesOfText(F("Analysing frequency."), F("f=0.000"),F("+-0.000 <"));
            isRecording = true;currentSample = 0;isDeviceIdle = false;
            startCommandReceived = true;calibrateCommandReceived = false;
            startSavingCommandReveived = false;isRecording = true;
            scanModecVEP = false;scanModeSSVEP = true;
            startTime = micros();  // Reset timer
            }
            else{
              Serial.println(F("No sensors detecteed."));
              displayTwoLinesOfText(F("No sensor"), F("data detected!!!"));
              isDeviceIdle = true;
            }
        }
        break;

      case 2:  // Start/Stop Mode cvep anaylsis
        if (isRecording) {
          Serial.println(F("Stop command received."));
          
          startCommandReceived = false;
          calibrateCommandReceived = false;
          startSavingCommandReveived = false;
          isRecording = false;
          forceStop = false;
          scanModecVEP = false;
          scanModeSSVEP = false;

          unsigned long startTime = millis();  // Record start time
          uint8_t tapPoint = 0;uint8_t tapPoint2 = 0;
          uint8_t detectedPolynomial = 0;uint8_t detectedPolynomial2 = 0;
          // Perform the calculations
          for (int i = 0; i < 48; i++) {
            //Serial.print(24+i);
            if(sensorCheck1 && sensorCheck2){ // all sensors are connected
              downSample32(sensorData1, reData, byteSize, 24 + i);  // Downsample data
              tapPoint = compareSequences32(reData, (24 + i) * 10, 30);
              downSample32(sensorData2, reData2, byteSize, 24 + i);  // Downsample data
              tapPoint2 = compareSequences32(reData2, (24 + i) * 10, 30);
            }
            else if(sensorCheck1 ^ sensorCheck2) { // single sensor connected
              downSample32(sensorData1, reData, byteSize, 24 + i);  // Downsample data
              tapPoint = compareSequences32(reData, (24 + i) * 10, 30);
              tapPoint2 = 0;
            }
            
            if (tapPoint != 0b00000000)    // Compare sequences
            {
              Serial.println("Sensor1: ");Serial.print(24 + i);
              Serial.print(" hz, ");Serial.println(tapPoint,BIN);
              Serial.println(getPolynomial(tapPoint));
              if((detectedPolynomial != tapPoint) && (detectedPolynomial == 0)){
                  detectedPolynomial = tapPoint;
              }
              else if((detectedPolynomial == tapPoint) && (detectedPolynomial != 0)){
                  detectedPolynomial = tapPoint;
              }
            }
            if(tapPoint2 != 0b00000000){
              Serial.println("Sensor2: ");Serial.print(24 + i);
              Serial.print(" hz, ");Serial.println(tapPoint2,BIN);
              Serial.println(getPolynomial(tapPoint2));
              if((detectedPolynomial2 != tapPoint2) && (detectedPolynomial2 == 0)){
                  detectedPolynomial2 = tapPoint2;
              }
              else if((detectedPolynomial2 == tapPoint2) && (detectedPolynomial2 != 0)){
                  detectedPolynomial2 = tapPoint2;
              }

            }
            
          }

          unsigned long elapsedTime = millis() - startTime;  // Calculate elapsed time

          // Print the elapsed time
          Serial.print(F("Calculation Time: "));
          Serial.print(elapsedTime);
          Serial.println(F(" ms"));
          // HANDLE DETECTED POLYNOMIALS
          if(detectedPolynomial != 0 && (sensorCheck1 ^ sensorCheck2)) // single sensor connected and a polynomial detected
          {
            displayTwoLinesOfText(F("Polynomial Detected!"), getPolynomial(detectedPolynomial));
          }
          else if((sensorCheck1 && sensorCheck2) ) // both sensors connected and at least one polynomial detected && ( detectedPolynomial || detectedPolynomial2)
          {
              if(detectedPolynomial != 0 && detectedPolynomial2 == 0 )
               displayTwoLinesOfText("s1:"+  getPolynomial(detectedPolynomial) , "s2: NaN" );
              else if(detectedPolynomial == 0 && detectedPolynomial2 != 0 )
               displayTwoLinesOfText("s1: NaN" , "s2:"+  getPolynomial(detectedPolynomial2) );
              else
               displayTwoLinesOfText("s1:"+  getPolynomial(detectedPolynomial) , "s2:"+  getPolynomial(detectedPolynomial2) );
          }
          else   // nothing detected at all
          {displayTwoLinesOfText(F("No Polynomial Detected!"), F(":("));}
          // TEST TODO
          //resetSensorData();
          isDeviceIdle = true;
        } else { // START
          currentSample = 0;
          resetSensorData();
          if(sensorCheck1 || sensorCheck2){
                enableSensorPower(enSens);
               enableSensorPower(enSens2);
            Serial.println(F("Start command received."));
              displayTwoLinesOfText(F("Looking for"), F("m-sequences."));
              isRecording = true;
              currentSample = 0;  //reset location
              isDeviceIdle = false;
              startCommandReceived = true;
              calibrateCommandReceived = false;
              startSavingCommandReveived = false;
              isRecording = true;
              scanModecVEP = true;
              scanModeSSVEP = false;
              startTime = micros();  // Reset timer
              }
              else{
                disableSensorPower(enSens);
                disableSensorPower(enSens2);
                Serial.println(F("No sensors detecteed."));
              displayTwoLinesOfText(F("No sensor"), F("data detected!!!"));
              isDeviceIdle = true;
              }
       }
        break;
        
      case 4:
          if(onBoardStimulusEnabled){
            disableTestingLED(pwmPin3);
            onBoardStimulusEnabled = false;
            displayThreeLinesOfText(F("Toggle"), F("Internal Stimuli"),"Off");
          }
          else{
            enableTestingLED(pwmPin3);
            onBoardStimulusEnabled = true;
            displayThreeLinesOfText(F("Toggle"), F("Internal Stimuli"),"On");
          }       
      break;
      
    }
  }

  

}


void loop() {
  if (Serial.available() > 0) {
    checkSerialCommands();  // function dealing with terminal commands from pc just for simplifying the main.
    idleCounter = 0;
  }

  if(initialStart){
    delay(350);
    batteryStatus =  "Bat: " + String(measureBattery()) + " %";
    //displayTwoLinesOfText(F("Main Menu"), batteryStatus);
    switch (currentMode) {
      case 0:
        batteryStatus =  "Bat: " + String(measureBattery()) + " %";
        Serial.println(F("Mode: Main Menu"));
        displayTwoLinesOfText(F("Main Menu"), batteryStatus);
        break;
      // case 1:
      //   Serial.println(F("Mode: Calibration"));
      //   displayTwoLinesOfText(F("Calibration Mode"), F(""));
      //   break;
      case 3:
        Serial.println(F("Stream Mode: Start/Stop"));
        displayTwoLinesOfText(F("Start/Stop Mode"),  F("Data Stream"));
        break;
      case 1:
        Serial.println(F("SSVEP Mode: Start/Stop"));
        displayTwoLinesOfText(F("Start/Stop Mode"), F("Find frequency."));
        break;
      case 2:
        Serial.println(F("cVEP Mode: Start/Stop "));
        displayTwoLinesOfText(F("Start/Stop Mode"), F("Find polynomial of\nthe m-sequence."));
        break;
      case 4:
        Serial.println(F("Mode: Internal Stimuli"));
        //displayThreeLinesOfText(F("Toggle"), F("On board light stimulus."),"");
        if(onBoardStimulusEnabled)
          displayThreeLinesOfText(F("Toggle"), F("Internal Stimuli"),"On");
        else
          displayThreeLinesOfText(F("Toggle"), F("Internal Stimuli"),"Off");
        break;
    }
    initialStart = false;

    Serial.println("");
    Serial.println(EEPROM.read(0));
    Serial.println(EEPROM.read(1));
    Serial.println(EEPROM.read(2));
    Serial.println(EEPROM.read(3));
    Serial.println(EEPROM.read(4));
  }
  checkButtonCommands();
 
  //Data loop
  // Data loop
  if (startCommandReceived) {
        unsigned long currentMicros = micros();
        unsigned long elapsedTime = currentMicros - startTime; // Calculate elapsed time

      
        if (scanModecVEP && elapsedTime >= 10500000UL) {  //CVEP analysis termination
          Serial.println(F("cVEP mode: Scanning complete after 10.5 seconds."));
          startCommandReceived = false; // Stop scanning
          stopAndAnalyseCVEP();
          //return;
        }

        
        if (scanModeSSVEP) { // SSVEP analysis simple frequency analysis
          static unsigned long lastEdgeTime = 0;
          
          // On each loop iteration:
          int currentPinValue;

          if (sensorCheck2 && !sensorCheck1)
            currentPinValue = digitalRead(inputPin2); 
          else
            currentPinValue = digitalRead(inputPin);

          // Rising edge detection
          static int prevPinValue = LOW;
          if (currentPinValue == HIGH && prevPinValue == LOW) {
              unsigned long thisEdgeTime = micros();
              if (lastEdgeTime != 0 && (thisEdgeTime > lastEdgeTime)) {
                unsigned long edgeDelta = thisEdgeTime - lastEdgeTime;// how much time passed
                double frequency = 0;
                
                if((1000000.0 / (double)edgeDelta) <= communicationSpeed/2) // ignore frequencies higher than half the sampling frequency
                  frequency = 1000000.0 / (double)edgeDelta;
                else
                  frequency = measuredFrequency;

                
                // Optimized EWMA
                //measuredFrequency = (frequency + 15.0 * measuredFrequency + 0.5) / 16.0;
                //measuredFrequency = 0.2 * ( 2 * measuredFrequency + 4 * frequency);  // simple weighted average filter for more stable output
                measuredFrequency = 0.8 * measuredFrequency + 0.2 * frequency;
                //measuredFrequency = frequency;
               
                

              }
              lastEdgeTime = thisEdgeTime;
          }
          prevPinValue = currentPinValue;

          static unsigned long lastPrintTime = 0;
          unsigned long now = millis();
          if (now - lastPrintTime > 500) {   // Print every half-second
              Serial.print(F("Frequency Detected: "));
              Serial.print(measuredFrequency, 4);
              Serial.print(F(" Hz "));
              Serial.print(F("+-"));
              Serial.println(standardDeviation(freqArray, recordedAmount),4);
              freqArray[freqArrayIndex] = measuredFrequency;
              freqArrayIndex = (freqArrayIndex + 1) % 10; // loop between 0-9 
              if(recordedAmount < 10)
              {
                recordedAmount++;
              }
              //displayTwoLinesOfText("Frequency analysis:", "\nf= " + String(measuredFrequency, 4) + " Hz");
              displayThreeLinesOfText("Frequency analysis:", "f= " + String(measuredFrequency, 4) + " Hz" , "+-" + String(standardDeviation(freqArray, recordedAmount),4) + " ||");
              lastPrintTime = now;
          }
        }


    if ((sensorCheck1 != 0) && (sensorCheck2 != 0)) { // Dual sensors detected
      pinValue = digitalRead(inputPin);
      pinValue2 = digitalRead(inputPin2);
      forceStop = false;
    } else if ((sensorCheck1 == 0) && (sensorCheck2 != 0)) { // Only sensor 2 detected
      pinValue = digitalRead(inputPin2);
      pinValue2 = 0;
      forceStop = false;
    } else if ((sensorCheck1 != 0) && (sensorCheck2 == 0)) { // Only sensor 1 detected
      pinValue = digitalRead(inputPin);
      pinValue2 = 0;
      forceStop = false;
    } else { // Nothing detected
      Serial.println(F("No sensor detected."));
      pinValue = 0;
      pinValue2 = 0;
      forceStop = true;
      return;
    }

    if (currentMicros - previousMicros >= communicationSpeed) {
      previousMicros = currentMicros;
      unsigned long timestamp = currentMicros - startTime;
      if(!scanModeSSVEP && !scanModecVEP)
      {Serial.print(timestamp, HEX);  // Print timestamp
      Serial.print(F(","));}
      
      // Print sensor data
      if ((sensorCheck1 != 0) && (sensorCheck2 != 0)) { // Dual sensors
        if(!scanModeSSVEP && !scanModecVEP) {
          Serial.print(pinValue);
          Serial.print(F(","));
          Serial.println(pinValue2);}
        if (currentSample < 10016) {
          setBit32(sensorData1, currentSample, pinValue);
          setBit32(sensorData2, currentSample, pinValue2);
          currentSample++;
        }
      } else { // Single sensor
        
        if(!scanModeSSVEP && !scanModecVEP)
          Serial.println(pinValue);
        if (currentSample < 10016) {
          setBit32(sensorData1, currentSample, pinValue);
          currentSample++;
        }
      }
    }
  }


  if (calibrateCommandReceived) {
    isDeviceIdle = false;
    enableSensorPower(enSens);
    enableSensorPower(enSens2);
    displayTwoLinesOfText(F("Calibrating..."), F(""));
    if(testSensorConnection(pwmPin, inputPin, fixedDutyCycle, sensorCheck1) > 0) //test for basic sensor outputs
      {fixedDutyCycle = calibrateSensor(pwmPin, inputPin, fixedDutyCycle, sensorCheck1 );  //calibrate();
      EEPROM.write(0, fixedDutyCycle);
      EEPROM.write(2, 1);
      } // save sensor1 state
    else{
      EEPROM.write(0, 0); // save sensor1 state
      EEPROM.write(2, 0);
      disableSensorPower(enSens);
      disableSensorPower(enSens2);
    }
    
    if(testSensorConnection(pwmPin2, inputPin2, fixedDutyCycle2, sensorCheck2) > 0)  //test for basic sensor outputs
      {fixedDutyCycle2 = calibrateSensor(pwmPin2, inputPin2, fixedDutyCycle2, sensorCheck2);
      EEPROM.write(1, fixedDutyCycle2);
      EEPROM.write(3, 1);
      } // save sensor2 state
    else{
      EEPROM.write(1, 0); // save sensor2 state
      EEPROM.write(3, 0);
      disableSensorPower(enSens);
      disableSensorPower(enSens2);
    }

    char line1[32], line2[32];
    if(sensorCheck1 > 0)
      sprintf(line1, "Sensor 1: %d", fixedDutyCycle);
    else
      sprintf(line1, "Sensor 1: ---");
    if(sensorCheck2 > 0)
      sprintf(line2, "Sensor 2: %d", fixedDutyCycle2);
    else
       sprintf(line2, "Sensor 2: ---");
    displayTwoLinesOfText(line1, line2);
    calibrateCommandReceived = false;
    isDeviceIdle = true;
    idleCounter = 0;
    EEPROM.commit();// push changes to memory
  }
  
  if (isDeviceIdle) { //IDLE COUNTER LOOP
    currentTime = millis();
    if (currentTime - lastIncrementTime >= 1) { // If one millisecond has passed update counter
      idleCounter = idleCounter + 1;
      lastIncrementTime = currentTime;
      //Serial.println(idleCounter);
    }

    if ((idleCounter >= idleThreshold) && isDeviceIdle) {
      displayTwoLinesOfText("","");
      Serial.println("Zzzzzzz...");
      disableSensorPower(enSens);
      disableSensorPower(enSens2);
      disableTestingLED(pwmPin3);
      pinMode(enSens, INPUT);pinMode(enSens2, INPUT);pinMode(pwmPin3, INPUT);
      esp_deep_sleep_start();// Enter deep sleep
      lastIncrementTime = currentTime;
      idleCounter = idleThreshold + 1; //no need to count further (prevents overflow)
   } 
  //  else {
  //    idleCounter = 0;
  //    lastIncrementTime = currentTime;
  //  }

  } else { //reset timer
    idleCounter = 0;
    lastIncrementTime = currentTime;
    //Serial.println("reset");
  }
  
}
