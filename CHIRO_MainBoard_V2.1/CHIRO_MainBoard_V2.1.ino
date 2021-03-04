
// V2.1 Modifications:
// * stay on for 1 hour
// * 1/2 sensitivity level for red, 1/3 for blue and green
// * only use amp
// * faster startup
// display low battery during normal loop (Not yet implemented)
// * default to blue (the old red)

// todo: 
// finish fixing ads1220 library, move it to this project
// Comments
// STRETCH GOAL add separate temp sensor mode (modes 4-7) (possibly includes extracting temp data from ADC)

// WARNING ABOUT THE ADS1220 LIBRARY:
// I have modified the library, as of the writing of this code the library had a bad bug that
// prevented useful reads from occuring.

#include <WS2812Serial.h> 
#include "GlobalVariables.h"
#include <ADC.h>
#include "Utils.h"

void setup() {
  setupPinModes();

  lngHeartbeatTimer = millis();
  lngShutOffTimer = millis();

  //externalADC.begin(pinADS1220CS,pinADS1220DRDY);
  //externalADC.set_data_rate(DR_175SPS);
  //externalADC.set_pga_gain(PGA_GAIN_128);
  //externalADC.set_conv_mode_single_shot();
  //externalADC.select_mux_channels(MUX_AIN0_AIN1);

  configureADCs();

  modeButton.setCountMode(COUNT_PRESSING);
  modeButton.setDebounceTime(50);
  
  leds.begin();
  leds.clear();
  leds.show();
  
  Serial.begin(1000000);

  delay(500);
  checkBatteryLevel();
  Serial.print("Battery Voltage: "); Serial.println(fltBatSense);
  animateBatteryVoltageOnLEDs();
  checkShutOffConditions();
  loadCalibration();
  //calibrateSensorOffsets();
  
  delay(500);
  Serial.println("Setup Finished!");
}

void loop() {
  lngLoopTimerStart = millis();
  if (millis() - lngShutOffTimer > 3600000){ // 300000){ // 5 minutes // 500 minutes
    Serial.println("TIME LIMIT REACHED, WE ARE TURNING OFF");
    digitalWrite(pinSTAYON, LOW); // shut off, not reversible without human interaction
  }

  updateHeartbeat();

  sampleADCs();

  //delay(300); // not needed with the sigma delta adc- it takes long enough to perform conversions!

  checkButtons();

  lngLoopTimerEnd = millis();
  
  checkBatteryLevel();
  checkShutOffConditions();

  

  updateLEDs();
  updateAudio();

  

  //plotTiming();

}

void plotTiming(){
  LNGloopTimer[intLoopTimerPointer] = lngLoopTimerEnd-lngLoopTimerStart;
  intLoopTimerPointer ++;
  if (intLoopTimerPointer > 499){
    for (int i = 0; i < 500; i ++){
      Serial.println(LNGloopTimer[i]);
    }
    intLoopTimerPointer = 0;
  }
}

void animateBatteryVoltageOnLEDs(){
  intLedPosition = (int)((fltBatSense-2.8)*17.85); // map 2.8V-4.2V to 25 pixels
  if(intLedPosition>24){intLedPosition=24;}
  leds.clear();
  for(int i = 0; i<=intLedPosition;i++){
    if (i<6){ledsSetChannel(i,RED_CHANNEL,pgm_read_byte(&gamma8[(byte)intMaxBrightness/2]));}
    else {ledsSetChannel(i,GREEN_CHANNEL,pgm_read_byte(&gamma8[(byte)intMaxBrightness/2]));}
    // no gamma correction here, since it is destructive!
    leds.show();
    delay(20);
  }
  delay(300);
}

void updateAudio(){
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
}

void addNeedleGaugeChannel(float barGraphValue, int colorChannel){
  intLedPosition = (int)barGraphValue;
  intLedIntensity = (int)((abs(barGraphValue)-abs((float)intLedPosition))*intMaxBrightness);
  if (barGraphValue > 12){
    intLedPosition = 12;
    intLedIntensity=0;
  } else if (barGraphValue < -12){
    intLedPosition = -12;
    intLedIntensity=0;
  }
  intLedPosition += 12;
  ledsSetChannel(intLedPosition,colorChannel,intMaxBrightness-intLedIntensity);
  if (barGraphValue > 0){ledsSetChannel(intLedPosition+1,colorChannel,intLedIntensity);}
  else {ledsSetChannel(intLedPosition-1,colorChannel,intLedIntensity);}  
}

void updateLEDs(){
  leds.clear();
  //if (intMode == 0 || intMode == 3){addNeedleGaugeChannel(fltTempDiffAve*3.0, GREEN_CHANNEL);ledsSetChannel(25, GREEN_CHANNEL,intMaxBrightness);}
  //if (intMode == 1 || intMode == 3){addNeedleGaugeChannel(fltTempDiffAve, BLUE_CHANNEL);ledsSetChannel(25, BLUE_CHANNEL,intMaxBrightness);}
  //if (intMode == 2 || intMode == 3){addNeedleGaugeChannel(fltTempDiffAve/3.0, RED_CHANNEL);ledsSetChannel(25, RED_CHANNEL,intMaxBrightness);}
  //if (intMode == 4 || intMode == 7){addNeedleGaugeChannel(fltTempDiffAveAmp*3.0, GREEN_CHANNEL);ledsSetChannel(26, GREEN_CHANNEL,intMaxBrightness);}
  //if (intMode == 5 || intMode == 7){addNeedleGaugeChannel(fltTempDiffAveAmp, BLUE_CHANNEL);ledsSetChannel(26, BLUE_CHANNEL,intMaxBrightness);}
  //if (intMode == 6 || intMode == 7){addNeedleGaugeChannel(fltTempDiffAveAmp/3.0, RED_CHANNEL);ledsSetChannel(26, RED_CHANNEL,intMaxBrightness);}

  //if (intMode == 0 || intMode == 3){addNeedleGaugeChannel(fltTempDiffAve, GREEN_CHANNEL);ledsSetChannel(25, GREEN_CHANNEL,intMaxBrightness);}
  //if (intMode == 1 || intMode == 3){addNeedleGaugeChannel(fltTempDiffAve/3.0, BLUE_CHANNEL);ledsSetChannel(25, BLUE_CHANNEL,intMaxBrightness);}
  //if (intMode == 2 || intMode == 3){addNeedleGaugeChannel(fltTempDiffAve/9.0, RED_CHANNEL);ledsSetChannel(25, RED_CHANNEL,intMaxBrightness);}

  // For Type E
  if (intMode == 2 || intMode == 3){addNeedleGaugeChannel(fltTempDiffAveAmp/3.0, GREEN_CHANNEL);ledsSetChannel(26, GREEN_CHANNEL,intMaxBrightness);}
  if (intMode == 0 || intMode == 3){addNeedleGaugeChannel(fltTempDiffAveAmp/9.0, BLUE_CHANNEL);ledsSetChannel(26, BLUE_CHANNEL,intMaxBrightness);}
  if (intMode == 1 || intMode == 3){addNeedleGaugeChannel(fltTempDiffAveAmp/18.0, RED_CHANNEL);ledsSetChannel(26, RED_CHANNEL,intMaxBrightness);}

  // for type T
  //if (intMode == 2 || intMode == 3){addNeedleGaugeChannel(fltTempDiffAveAmp/1.66, GREEN_CHANNEL);ledsSetChannel(26, GREEN_CHANNEL,intMaxBrightness);}
  //if (intMode == 0 || intMode == 3){addNeedleGaugeChannel(fltTempDiffAveAmp/5.0, BLUE_CHANNEL);ledsSetChannel(26, BLUE_CHANNEL,intMaxBrightness);}
  //if (intMode == 1 || intMode == 3){addNeedleGaugeChannel(fltTempDiffAveAmp/10.0, RED_CHANNEL);ledsSetChannel(26, RED_CHANNEL,intMaxBrightness);}
  
  if (booLowBatLEDState == true){ledsSetChannel(25, RED_CHANNEL,intMaxBrightness);}
  gammaCorrect();
  leds.show();
  if (booModeAudio == true){digitalWrite(pinAUDIOLED,LOW);}
  else{digitalWrite(pinAUDIOLED,HIGH);}
}
