

#ifndef Utils_h
#define Utils_h

#include <Arduino.h>
#include "GlobalVariables.h"
#include <ADC.h>
#include <EEPROM.h>

void printCalibrationOffsets(){
  Serial.print("Calibrated Differential Offset (external ADC): "); Serial.print(fltADCDiffOffset); Serial.println("uV");
  Serial.print("Calibrated Differential Offset (internal ADC + amp): "); Serial.print(fltADCDiffOffsetAmp); Serial.println("uV");
}

void loadCalibration(){
  EEPROM.get(120, fltADCDiffOffset);
  EEPROM.get(130, fltADCDiffOffsetAmp);
  Serial.println("Loading Sensor Calibration Offsets!");
  printCalibrationOffsets();
}

void saveCalibration(){
  Serial.println("Saving Sensor Calibration Offsets!");
  EEPROM.put(120, fltADCDiffOffset);
  EEPROM.put(130, fltADCDiffOffsetAmp);  
}

void updateHeartbeat(){
  if (millis() - lngHeartbeatTimer > 1000){
    booHeartbeatState = !booHeartbeatState;
    digitalWrite(pinHEARTBEAT, booHeartbeatState);
    lngHeartbeatTimer = millis();
  }
}

void configureADCs(){
  // Sampling speed is how long the ADC waits for the internal sample and hold cap to equilize. 
  // For a large, low impedance external cap (.1u or larger), this can be very small (very fast).
  // Conversion Speed is what base speed is used for the ADC clock. It influences sampling time 
  // (which is set in number of adc clocks), as well as how fast the conversion clock runs.
  // Resolution *MAY* influence how many adc clocks are needed to make a conversion. 
  // Averaging is how many samples to take and average.
  adc->setReference(ADC_REFERENCE::REF_3V3, ADC_0); // options are: ADC_REFERENCE::REF_3V3, ADC_REFERENCE::REF_EXT, or ADC_REFERENCE::REF_1V2.
  adc->setSamplingSpeed(ADC_SAMPLING_SPEED::LOW_SPEED); // options are VERY_LOW_SPEED, LOW_SPEED, MED_SPEED, HIGH_SPEED or VERY_HIGH_SPEED.
  adc->setConversionSpeed(ADC_CONVERSION_SPEED::LOW_SPEED); // options are VERY_LOW_SPEED, LOW_SPEED, MED_SPEED, HIGH_SPEED_16BITS, HIGH_SPEED or VERY_HIGH_SPEED. VERY_HIGH_SPEED may be out of specs
  adc->setAveraging(64); // set number of averages
  adc->setResolution(intADCResolution); // set number of bits 

  adc->setReference(ADC_REFERENCE::REF_3V3, ADC_1);
  adc->setSamplingSpeed(ADC_SAMPLING_SPEED::LOW_SPEED, ADC_1);
  adc->setConversionSpeed(ADC_CONVERSION_SPEED::LOW_SPEED, ADC_1);
  adc->setAveraging(64, ADC_1); 
  adc->setResolution(intADCResolution, ADC_1);  
}

float convertADCToMicroV(int32_t i32data)
{
    return (float)((i32data*VFSR*1000000)/FULL_SCALE);
}

void computeAverages(){
  fltTempDiffAve = 0;
  fltTempDiffAveAmp = 0;
  for(int i = 0; i<NUMAVERAGES; i++){
    fltTempDiffAve += FLTtempDiffHist[i];
    fltTempDiffAveAmp += FLTtempDiffHistAmp[i];
  }
  fltTempDiffAve = fltTempDiffAve/NUMAVERAGES;
  fltTempDiffAveAmp = fltTempDiffAveAmp/NUMAVERAGES;
}

void ledsSetChannel(int pixel, int channel, int value){
  drawingMemory[(pixel*3)+channel]=(byte)value;
}

void setupPinModes(){
  pinMode(pinSTAYON, OUTPUT);
  digitalWrite(pinSTAYON, HIGH); // needs to happen right away or we shut off!
  pinMode(pinHEARTBEAT, OUTPUT);
  pinMode(pinAUDIOLED, OUTPUT);
  digitalWrite(pinAUDIOLED, HIGH);
  pinMode(pinBUZZER, OUTPUT);
  digitalWrite(pinBUZZER, LOW);
  pinMode(pinMODEBUTTON, INPUT);
  analogWriteResolution(8);
  analogWriteFrequency(pinBUZZER, 400);
}

void sampleADCs(){
  //Serial.println(analogRead(pinANALOGDIFFAMP)); // to use the old analog amp input
  if (intMode<4 || booCalibration){
    externalADC.select_mux_channels(MUX_AIN0_AIN1);
    delayMicroseconds(100);
    adc_data1 = externalADC.Read_SingleShot_WaitForData();
    fltThermocouple1Voltage = convertADCToMicroV(adc_data1);
    externalADC.select_mux_channels(MUX_AIN2_AIN3);
    delayMicroseconds(100);
    adc_data2 = externalADC.Read_SingleShot_WaitForData();
    fltThermocouple2Voltage = convertADCToMicroV(adc_data2);
    fltTempDiff = fltThermocouple1Voltage + fltThermocouple2Voltage; // addition because one is negative
    fltTempDiff -= fltADCDiffOffset;
    FLTtempDiffHist[intTempDiffHistPointer] = fltTempDiff;
  }
  if (intMode>3 || booCalibration){
    lngAmpRaw = 0;
    for (int i = 0; i < 64; i ++){lngAmpRaw += adc->analogRead(pinANALOGDIFFAMP);}
    lngAmpRaw = lngAmpRaw/64;
    fltTempDiffAmp = -((((float)lngAmpRaw)/((float)pow(2,intADCResolution))*1000*3.3)-(3.3*1000/2));
    fltTempDiffAmp -= fltADCDiffOffsetAmp;
    FLTtempDiffHistAmp[intTempDiffHistPointer] = fltTempDiffAmp;
  }
  
  //Serial.print("Thermocouple Voltages: "); Serial.print(fltThermocouple1Voltage);Serial.print(", ");
  //Serial.print(fltThermocouple2Voltage); Serial.println();
  //Serial.print(lngAmpRaw);Serial.print(",");Serial.print(fltTempDiffAmp);Serial.println();

  intTempDiffHistPointer ++;
  if (intTempDiffHistPointer>NUMAVERAGES-1){intTempDiffHistPointer=0;}
  computeAverages();
}

void calibrateSensorOffsets(){
  booCalibration = true;
  leds.clear();
  leds.setPixel(0,GREEN); leds.setPixel(4,BLUE); leds.setPixel(8,RED); leds.setPixel(12,WHITE); 
  leds.setPixel(16,RED); leds.setPixel(20,BLUE);  leds.setPixel(24,GREEN);
  leds.show();
  delay(500);
  fltADCDiffOffset = 0;
  fltADCDiffOffsetAmp = 0;
  for (int i = 0; i < NUMAVERAGES; i ++){
    sampleADCs();
  }
  fltADCDiffOffset = fltTempDiffAve;
  fltADCDiffOffsetAmp = fltTempDiffAveAmp;
  printCalibrationOffsets();
  delay(500);
  saveCalibration();
  booCalibration = false;
}

void checkButtons(){
  modeButton.update();
  if (modeButton.getPressingEdge()==true){ // button pressed!
    lngShutOffTimer = millis(); // reset the turn-off timer, since we just used a button
    intMode += 1; if (intMode > 7){intMode = 0;}
    Serial.println("Mode Button Pressed!");
    booCheckButtonHeld = true;
    if (modeButton.getDoubleTap() == true){
      Serial.println("Double tap detected, running calibration routine! (and backing up 2 modes)");
      intMode -= 1;  if (intMode < 0){intMode = 7;}
      intMode -= 1;  if (intMode < 0){intMode = 7;}
      calibrateSensorOffsets();
    }
  }
  if (modeButton.getStateTime() > 2000 && booCheckButtonHeld == true && modeButton.getState() == 1){
    booCheckButtonHeld = false;
    booModeAudio = !booModeAudio;
    Serial.println("Audio State Changed! (backing up 1 mode too, since the user didn't mean to change mode).");
    intMode -= 1;  if (intMode < 0){intMode = 7;}
  }
  if (modeButton.getStateTime() > 4000 && modeButton.getState() == 1){
    digitalWrite(pinSTAYON, LOW); // shut off, not reversible without human interaction
  }
  if (modeButton.getState() == 0){booCheckButtonHeld = false;}
}

// DESTRUCTIVELY gamma corrects all LEDs. Only works if you re-render from scratch every frame!
// see other projects for a non-destructive version (that requires another LED buffer)
void gammaCorrect(){
  for (int i = 0; i < numled*3; i++){
    drawingMemory[i] = pgm_read_byte(&gamma8[drawingMemory[i]]);
  }
}

void checkBatteryLevel(){
  lngBatSense = adc->analogRead(pinBATTERYVSENSE);
  fltBatSense = 3.3*2*((float)lngBatSense)/((float)pow(2,intADCResolution)); // 3.3 is Vref, 2 is resistor divider scalar
  //Serial.print("Battery Voltage: "); Serial.print(fltBatSense); Serial.println(" Volts.");
}

void checkShutOffConditions(){
  if (fltBatSense < 2.95){digitalWrite(pinSTAYON, LOW);} // shut off, not reversible without human interaction
}

#endif
