
// V2.1 Modifications:
// * stay on for 1 hour
// * 1/2 sensitivity level for red, 1/3 for blue and green
// * only use amp
// * faster startup
// display low battery during normal loop (Not yet implemented)
// * default to blue (the old red)

// V3 updates:
// moving to new hardware, ATMEGA (for automated assembly) with only amplifiers, onboard adc,
  // and using discrete LEDs with a port expander driver.

#include <SPI.h>              // We use this library within the MCP23S17 library, so it must be called here.
#include <MCP23S17.h> // https://github.com/n0mjs710/MCP23S17
#include <EEPROM.h>
#include "GlobalVariables.h"
#include "Utils.h"
#include "LEDs.h"

void setup() {
  setupPinModes();
  initializePortExpanders();
  initializeButtons();

  loadState();

  lngHeartbeatTimer = millis();
  lngShutOffTimer = millis();

  Serial.begin(38400);

  delay(500);
  checkBatteryLevel();
  Serial.print("Battery Voltage: "); Serial.println(fltBatSense);
  animateBatteryVoltageOnLEDs();
  checkShutOffConditions();
  loadCalibration();
  
  //calibrateSensorOffsets(); // optionally, uncomment this on a brand new board to force calibration. Or just click to calibrate first unit.
  
  delay(500);
  Serial.println("Setup Finished!");
}

void loop() {
  lngLoopTimerStart = millis();
  if (millis() - lngShutOffTimer > 3600000){ // 300000){ // 5 minutes // 500 minutes
    Serial.println("TIME LIMIT REACHED, WE ARE TURNING OFF");
    delay(100); // time to finish printing
    digitalWrite(pinSTAYON, LOW); // shut off, not reversible without human interaction
  }

  updateHeartbeat();

  sampleADCs();

  //delay(300); // not needed with the sigma delta adc- it takes long enough to perform conversions!

  checkButtons();

  //lngLoopTimerEnd = millis(); change the location of this line to control what is being timed
  
  checkBatteryLevel();
  checkShutOffConditions();

  updateLEDs();
  updateAudio();

  lngLoopTimerEnd = millis();

  if (booPlotTemps){Serial.print(fltTempDiffAveuV);Serial.print("uV, ");Serial.print(fltTempDiffAveC,3);Serial.println("C");}
  if (booPlotTiming){plotTiming();}
}

void plotTiming(){
  LNGloopTimer[intLoopTimerPointer] = lngLoopTimerEnd-lngLoopTimerStart;
  intLoopTimerPointer ++;
  if (intLoopTimerPointer > 49){
    for (int i = 0; i < 50; i ++){
      Serial.print(LNGloopTimer[i]);Serial.print(",");
    } Serial.println();
    intLoopTimerPointer = 0;
  }
}

// Is this what we want long term?
void animateBatteryVoltageOnLEDs(){
  intBarGraphValue = 1+((int)((fltBatSense-2.8)*5)); // map 2.8V-4.2V to 8 pixels. This is a range of 1.4V, so .2V per pixel, hence *5. always have 1 light on hence 1+
  lngBarGraphUpdateTimer = millis();
  //delay(300); // delay doesn't work here unless you update the LEDS first
}

// FIX THIS FUNCTION
void updateAudio(){
  // currently not supported, since PWM frequencies are a bit much to play with right now
  /*
  if (booModeAudio == true){digitalWrite(pinAUDIOLED,LOW);}
  else{digitalWrite(pinAUDIOLED,HIGH);}
  int audioLevel = 0;
  if (abs(fltTempDiffAve)>8 && intMode < 4 && booModeAudio == true){
    audioLevel = (int)20*(abs(fltTempDiffAve) - 8)+250;
    if (audioLevel > 1000){audioLevel = 1000;}
    analogWriteFrequency(pinBUZZER, audioLevel);
    analogWrite(pinBUZZER, 128);
  }else if (abs(fltTempDiffAveAmp)>8 && booModeAudio == true){
    audioLevel = (int)20*(abs(fltTempDiffAveAmp) - 8)+250;
    if (audioLevel > 1000){audioLevel = 1000;}
    analogWriteFrequency(pinBUZZER, audioLevel);
    analogWrite(pinBUZZER, 128);
  }else{
    analogWrite(pinBUZZER, 0);
  }
  */
}

void updateLEDs(){
  intNeedlePosition = int(2.0*fltTempDiffAveC/needleSensitivities[intSensitivity-1]); // X2 to map to full bits in our half-bit scale
  // adjust to single bits only
  if (intNeedlePosition%2 == 1 && intNeedlePosition > 0){intNeedlePosition --;}
  if (intNeedlePosition%2 == 1 && intNeedlePosition < 0){intNeedlePosition ++;}
  intNeedlePosition = -intNeedlePosition; // based on how we physically assembled units, need to invert this
  
  updateLEDDrivers();
}
