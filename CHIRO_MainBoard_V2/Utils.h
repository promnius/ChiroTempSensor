

#ifndef Utils_h
#define Utils_h

#include <Arduino.h>
#include "GlobalVariables.h"
#include <ADC.h>

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
  for(int i = 0; i<6; i++){
    fltTempDiffAve += FLTtempDiffHist[i];
  }
  fltTempDiffAve = fltTempDiffAve/6;
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
  externalADC.select_mux_channels(MUX_AIN0_AIN1);
  delay(2);
  adc_data1 = externalADC.Read_SingleShot_WaitForData();
  fltThermocouple1Voltage = convertADCToMicroV(adc_data1);
  externalADC.select_mux_channels(MUX_AIN2_AIN3);
  delay(2);
  adc_data2 = externalADC.Read_SingleShot_WaitForData();
  fltThermocouple2Voltage = convertADCToMicroV(adc_data2);

  //Serial.print("Thermocouple Voltages: "); Serial.print(fltThermocouple1Voltage);Serial.print(", ");
  //Serial.print(fltThermocouple2Voltage); Serial.println();

  fltTempDiff = fltThermocouple1Voltage + fltThermocouple2Voltage; // addition because one is negative

  FLTtempDiffHist[intTempDiffHistPointer] = fltTempDiff;
  intTempDiffHistPointer ++;
  if (intTempDiffHistPointer>5){intTempDiffHistPointer=0;}
  computeAverages();
}

void checkButtons(){
  modeButton.update();
  if (modeButton.getPressingEdge()==true){ // button pressed!
    lngShutOffTimer = millis(); // reset the turn-off timer, since we just used a button
    intMode += 1;
    if (intMode > 7){intMode = 0;}
    Serial.println("Mode Button Pressed!");
    booCheckButtonHeld = true;
  }
  if (modeButton.getStateTime() > 3000 && booCheckButtonHeld == true){
    booCheckButtonHeld = false;
    booModeAudio = !booModeAudio;
    Serial.println("Audio State Changed!");
  }
  if (modeButton.getStateTime() > 6000 && modeButton.getState() == 1){
    digitalWrite(pinSTAYON, LOW); // shut off, not reversible without human interaction
  }
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
  if (fltBatSense < 3.0){digitalWrite(pinSTAYON, LOW);} // shut off, not reversible without human interaction
}

#endif
