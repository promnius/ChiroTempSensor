
// todo: 
// time update rate- find ways to make it faster?
// double check battery math- a fully charged battery should give a full bar, 
  // low battery should give warning before shutting off
// test buzzer/ add to specific modes/ add audio light
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

  externalADC.begin(pinADS1220CS,pinADS1220DRDY);
  externalADC.set_data_rate(DR_45SPS);
  externalADC.set_pga_gain(PGA_GAIN_128);
  externalADC.set_conv_mode_single_shot();
  externalADC.select_mux_channels(MUX_AIN0_AIN1);

  configureADCs();

  modeButton.setCountMode(COUNT_PRESSING);
  modeButton.setDebounceTime(50);
  
  leds.begin();
  leds.clear();
  leds.show();
  
  Serial.begin(1000000);

  delay(500);
  checkBatteryLevel();
  animateBatteryVoltageOnLEDs();
  checkShutOffConditions();
  
  delay(500);
  Serial.println("Setup Finished!");
}

void loop() {
  if (millis() - lngShutOffTimer > 300000){ // 5 minutes
    digitalWrite(pinSTAYON, LOW); // shut off, not reversible without human interaction
  }

  updateHeartbeat();

  sampleADCs();

  //delay(300); // not needed with the sigma delta adc- it takes long enough to perform conversions!

  checkButtons();
  checkBatteryLevel();
  checkShutOffConditions();

  updateLEDs();
  updateAudio();
}

void animateBatteryVoltageOnLEDs(){
  intLedPosition = (int)((fltBatSense-3.0)*17.85); // map 2.8V-4.2V to 25 pixels
  if(intLedPosition>24){intLedPosition=24;}
  leds.clear();
  for(int i = 0; i<=intLedPosition;i++){
    if (i<6){ledsSetChannel(i,RED_CHANNEL,pgm_read_byte(&gamma8[(byte)intMaxBrightness/2]));}
    else {ledsSetChannel(i,GREEN_CHANNEL,pgm_read_byte(&gamma8[(byte)intMaxBrightness/2]));}
    // no gamma correction here, since it is destructive!
    leds.show();
    delay(50);
  }
  delay(2000);
}

void updateAudio(){
  int audioLevel = 0;
  if (abs(fltTempDiffAve)>8 && booAudioMode == true){
    audioLevel = (int)20*(abs(fltTempDiffAve) - 8)+50;
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
  if (intMode == 0 || intMode == 3){addNeedleGaugeChannel(fltTempDiffAve*3.0, GREEN_CHANNEL);ledsSetChannel(25, GREEN_CHANNEL,intMaxBrightness);}
  if (intMode == 1 || intMode == 3){addNeedleGaugeChannel(fltTempDiffAve, BLUE_CHANNEL);ledsSetChannel(25, BLUE_CHANNEL,intMaxBrightness);}
  if (intMode == 2 || intMode == 3){addNeedleGaugeChannel(fltTempDiffAve/3.0, RED_CHANNEL);ledsSetChannel(25, RED_CHANNEL,intMaxBrightness);}
  if (intMode == 4 || intMode == 7){addNeedleGaugeChannel(fltTempDiffAve*3.0, GREEN_CHANNEL);ledsSetChannel(26, GREEN_CHANNEL,intMaxBrightness);}
  if (intMode == 5 || intMode == 7){addNeedleGaugeChannel(fltTempDiffAve, BLUE_CHANNEL);ledsSetChannel(26, BLUE_CHANNEL,intMaxBrightness);}
  if (intMode == 6 || intMode == 7){addNeedleGaugeChannel(fltTempDiffAve/3.0, RED_CHANNEL);ledsSetChannel(26, RED_CHANNEL,intMaxBrightness);}
  gammaCorrect();
  leds.show();
  if (booModeAudio == true){digitalWrite(pinAUDIOLED,LOW);}
  else{digitalWrite(pinAUDIOLED,HIGH);}
}
