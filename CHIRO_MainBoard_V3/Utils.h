

#ifndef Utils_h
#define Utils_h

#include <Arduino.h>
#include "GlobalVariables.h"
#include <EEPROM.h>

void printCalibrationOffsets(){
  Serial.print("Calibrated Differential Offset (resting state offset from nominal midpoint, as measured by internal ADC and external Amp): "); Serial.print(fltADCDiffOffset); Serial.println("uV");
}

void saveState(){
  Serial.println("Saving State!");
  EEPROM.put(100, intBrightness); Serial.println("Current Brightness Setting: "); Serial.println(intBrightness);
  EEPROM.put(90, intSensitivity); Serial.println("Current Sensitivity Setting: "); Serial.println(intSensitivity);  
}

void loadState(){
  Serial.println("Loading State!");
  EEPROM.get(100, intBrightness); Serial.println("Current Brightness Setting: "); Serial.println(intBrightness);
  EEPROM.get(90, intSensitivity); Serial.println("Current Sensitivity Setting: "); Serial.println(intSensitivity);
}

void loadCalibration(){
  EEPROM.get(130, fltADCDiffOffset);
  Serial.println("Loading Sensor Calibration Offsets!");
  printCalibrationOffsets();
}

void saveCalibration(){
  Serial.println("Saving Sensor Calibration Offsets!");
  EEPROM.put(130, fltADCDiffOffset);  
}

void updateHeartbeat(){
  // for this application, HB is distracting. It's possible a shorter or less bright HB could still be useful.
  /*
  if (booLowBatIndicator == true){
    if (millis() - lngHeartbeatTimer > 500){booLowBatLEDState = true;}
    else {booLowBatLEDState=false;}
  }
  if (millis() - lngHeartbeatTimer > 1000){
    booHeartbeatState = !booHeartbeatState;
    digitalWrite(pinHEARTBEAT, booHeartbeatState);
    lngHeartbeatTimer = millis();
  }*/
}

// setup pinmodes and default states
void setupPinModes(){
  pinMode(pinSTAYON, OUTPUT);
  digitalWrite(pinSTAYON, HIGH); // needs to happen right away or we shut off!
  pinMode(pinHEARTBEAT, OUTPUT);
  pinMode(pinAUDIOLED, OUTPUT);
  digitalWrite(pinAUDIOLED, HIGH);
  pinMode(pinBUZZER, OUTPUT);
  digitalWrite(pinBUZZER, LOW);
  pinMode(pinBUTTONOFF, INPUT);
  pinMode(pinBUTTONBRIGHTNESSUP, INPUT);
  pinMode(pinBUTTONBRIGHTNESSDOWN, INPUT);
  pinMode(pinBUTTONSENSITIVITYUP, INPUT);
  pinMode(pinBUTTONSENSITIVITYDOWN, INPUT);
  //analogWriteResolution(8);
  //analogWriteFrequency(pinBUZZER, 400);
}

// average a bunch of adc readings
void computeAverages(){
  fltTempDiffAveuV = 0;
  for(int i = 0; i<NUMAVERAGES; i++){
    fltTempDiffAveuV += FLTtempDiffHist[i];
  }
  fltTempDiffAveuV = fltTempDiffAveuV/NUMAVERAGES;
  fltTempDiffAveC = fltTempDiffAveuV/fltThermocoupleCoefficient;
  //Serial.println(fltTempDiffAveuV);
}

// actually sample the temp sensor ADC inputs, and convert to useful units.
void sampleADCs(){
  lngAmpRaw = 0;
  for (int i = 0; i < numSamples; i ++){lngAmpRaw += analogRead(pinANALOGDIFFAMP);}
  lngAmpRaw = lngAmpRaw/(numSamples/4); // the /4 increases the ADC resolution by 2 bits, nominally. Whether or not it's real is up for debate.
  fltTempDiffRawV = ((((float)lngAmpRaw)/4096.0)*5.0)-2.5; // "4096" is the nominal resolution of the ADC. 5.0 is the ADC voltage rail. 2.5 is the nominal midpoint
  fltTempDiffuV = fltTempDiffRawV*(1000000.0/fltAmpGain); // 1million is conversion from V to uV.
  fltTempDiffuV -= fltADCDiffOffset;
  FLTtempDiffHist[intTempDiffHistPointer] = fltTempDiffuV;

  //Serial.println(fltTempDiffuV);
  
  intTempDiffHistPointer ++;
  if (intTempDiffHistPointer>NUMAVERAGES-1){intTempDiffHistPointer=0;}
  computeAverages();
}

// reset system calibration
void calibrateSensorOffsets(){
  intMCP0 = 0b1111111111111111;
  intMCP1 = 0b1111111111111111;
  intMCP2 = 0b1111010110101111; // turn on LEDs 1,3,6,8
  mask = 0b0000000000000001 << (4-intBrightness);
  intMCP2 = intMCP2 & ~mask;
  mcpLEDs0.pinMode(intMCP0);
  mcpLEDs1.pinMode(intMCP1);
  mcpLEDs2.pinMode(intMCP2);
  delay(500);
  fltADCDiffOffset = 0;
  for (int i = 0; i < NUMAVERAGES*2; i ++){ // the *2 gurantees we have all fresh readings, since we don't know where in the buffer we started
    sampleADCs();
  }
  fltADCDiffOffset = fltTempDiffAveuV;
  printCalibrationOffsets();
  delay(500);
  saveCalibration();
}

// scan all buttons for states and transitions, and take action as needed.
void checkButtons(){
  btnOff.update();
  btnBrightnessUp.update();
  btnBrightnessDown.update();
  btnSensitivityUp.update();
  btnSensitivityDown.update();

  if (btnOff.getReleasingEdge()==true){
    Serial.println("Turn-off button pressed, WE ARE TURNING OFF");
    delay(30); // enable prints to happen
    digitalWrite(pinSTAYON, LOW); // shut off, not reversible without human interaction
  }
  if (btnBrightnessUp.getReleasingEdge()==true){
    intBrightness += 1; if (intBrightness > 4) {intBrightness = 4;} // 4 is max brightness
    Serial.print("Brightness Increased. New brightness level: "); Serial.println(intBrightness);
    intBarGraphValue = intBrightness*2; lngBarGraphUpdateTimer = millis();
    saveState();
  }
  if (btnBrightnessDown.getReleasingEdge()==true){
    intBrightness -= 1; if (intBrightness < 1) {intBrightness = 1;} // 1 is min brightness
    Serial.print("Brightness Decreased. New brightness level: "); Serial.println(intBrightness);
    intBarGraphValue = intBrightness*2; lngBarGraphUpdateTimer = millis();
    saveState();
  }
  if (btnSensitivityUp.getReleasingEdge()==true){
    intSensitivity += 1; if (intSensitivity > 8) {intSensitivity = 8;} // 8 is max Sensitivity
    Serial.print("Sensitivity Increased. New Sensitivity level: "); Serial.println(intSensitivity);
    intBarGraphValue = intSensitivity; lngBarGraphUpdateTimer = millis();
    saveState();
  }
  if (btnSensitivityDown.getReleasingEdge()==true){
    intSensitivity -= 1; if (intSensitivity < 1) {intSensitivity = 1;} // 1 is min Sensitivity
    Serial.print("Sensitivity Decreased. New Sensitivity level: "); Serial.println(intSensitivity);
    intBarGraphValue = intSensitivity; lngBarGraphUpdateTimer = millis();
    saveState();
  }

  if (btnSensitivityDown.getState() == 1 && btnSensitivityUp.getState() == 1){ // both buttons pressed together
    Serial.println("both sensitivity buttons pressed together, running calibration routine!");
    calibrateSensorOffsets();
  }
  if (btnBrightnessDown.getState() == 1 && btnBrightnessUp.getState() == 1){ // both buttons pressed together
    Serial.println("both brightness buttons pressed together, toggling audio.");
    booModeAudio = !booModeAudio;
  }
}

// scan the ADC for battery voltage. BLOCKING!
void checkBatteryLevel(){
  lngBatSense = 0;
  for (int i = 0; i < 4; i ++){lngBatSense += analogRead(pinBATTERYVSENSE);}
  //lngBatSense = lngBatSense/(numSamples/4); // the /4 increases the ADC resolution by 2 bits, nominally. Whether or not it's real is up for debate.
  fltBatSense = (5.0*((float)lngBatSense))/(4096.0); // "4096" is the nominal resolution of the ADC. 5.0 is the ADC voltage rail.
  //Serial.print("Battery Voltage: "); Serial.print(fltBatSense); Serial.println(" Volts.");
}

// turn off the system if battery voltage is low. Note this is only
// for making decisions, it doesn't measure anything.
void checkShutOffConditions(){
  if (fltBatSense < 3.2){booLowBatIndicator=true;}
  else {booLowBatIndicator = false;}
  if (fltBatSense < 2.95){Serial.println("LOW BATTERY, WE ARE TURNING OFF"); digitalWrite(pinSTAYON, LOW);} // shut off, not reversible without human interaction
}

void initializeButtons(){
  btnOff.setCountMode(COUNT_PRESSING);
  btnOff.setDebounceTime(50);
  btnBrightnessUp.setCountMode(COUNT_PRESSING);
  btnBrightnessUp.setDebounceTime(50);
  btnBrightnessDown.setCountMode(COUNT_PRESSING);
  btnBrightnessDown.setDebounceTime(50);
  btnSensitivityUp.setCountMode(COUNT_PRESSING);
  btnSensitivityUp.setDebounceTime(50);
  btnSensitivityDown.setCountMode(COUNT_PRESSING);
  btnSensitivityDown.setDebounceTime(50);
  delay(100);
  btnOff.update(); // clear out some initial state mumbo jumbo
  btnBrightnessUp.update();
  btnBrightnessDown.update();
  btnSensitivityUp.update();
  btnSensitivityDown.update();
  mask = btnOff.getReleasingEdge();
  mask = btnBrightnessUp.getReleasingEdge();
  mask = btnBrightnessDown.getReleasingEdge();
  mask = btnSensitivityUp.getReleasingEdge();
  mask = btnSensitivityDown.getReleasingEdge(); // clear out some intial states
}



#endif
