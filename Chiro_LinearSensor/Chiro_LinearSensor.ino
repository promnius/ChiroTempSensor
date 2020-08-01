
#include <ADC.h>
ADC *adc = new ADC(); // adc object

// SENSOR VARIABLE DECLARATION
long SensorValues[64];
//long SensorVoltages[64]; // in uV
//long SensorResistance[64]; // in mOhms
float SensorTemps[64];
long SensorCalibrations[64];
long temporarySensorValues[64];
float temporarySensorTemps[64];
float SensorTempsPrint[448];

int sensorExpanderCounter = 0;

const float invBeta = 1.00 / 3380.00;   // replace "Beta" with beta of thermistor
const  float adcMax = 65536.00;
const float invT0 = 1.00 / 298.15;   // room temp in Kelvin
float  K, C, F;


// PIN DECLARATIONS
int pinANALOGSELECT0 = 6;
int pinANALOGSELECT1 = 7;
int pinANALOGSELECT2 = 8;
int pinsSENSORS[] = {15,16,17,18,19,20,21,22};
int pinHeartbeat = 5;
//int pinHeartbeat = 13;


// PROGRAM CONSTANTS

// GLOBAL VARIABLES
int sensorCounter = 0;

// TIMING VARIABLES
unsigned long lngHeartbeatTimer = 0;
boolean booHeartbeatState = false;

void setup() {
  pinMode(pinANALOGSELECT0, OUTPUT);
  pinMode(pinANALOGSELECT1, OUTPUT);
  pinMode(pinANALOGSELECT2, OUTPUT);
  pinMode(pinHeartbeat, OUTPUT);
  configureADCs();

  Serial2.begin(9600);

  delay(100);
  for (int counter = 0; counter < 8; counter ++){
    pinMode(pinsSENSORS[counter], INPUT);
    delay(100);
  }
  delay(100);
  lngHeartbeatTimer = millis();
  //CalibrateADCs();

  
}

void loop() {
  if (millis() - lngHeartbeatTimer > 1000){
    booHeartbeatState = !booHeartbeatState;
    digitalWrite(pinHeartbeat, booHeartbeatState);
    lngHeartbeatTimer = millis();
  }
  for(int i = 0; i < 8; i ++){
    setMux(i);
    sampleSensors(i);
  }
  //applyCalibrations();
  fixSensorOrder();
  computeTemps();
  smoothTemps();
  delay(30);
  //printSensorData();
  //printSensorDataTemps();
  printSensorDataTempsExpanded();
  sendGaugeReading();
}

float maxValueLeft = 0.0;
float maxValueRight = 0.0;
int percentageInt = 0;
byte percentageByte = 0;

void sendGaugeReading(){
  maxValueLeft = 0.0;
  for (int i = 5; i < 28; i++){
    if (maxValueLeft < SensorTemps[i]){maxValueLeft = SensorTemps[i];}
  }
  maxValueRight = 0.0;
  for (int i = 33; i < 60; i++){
    if (maxValueRight < SensorTemps[i]){maxValueRight = SensorTemps[i];}
  }
  percentageInt = (int)(20*(maxValueLeft-maxValueRight));
  percentageInt += 128;
  if (percentageInt>255){percentageInt = 255;}
  if (percentageInt<0){percentageInt = 0;}
  percentageByte = (byte)percentageInt;
  //Serial.println(maxValueLeft-maxValueRight);
  Serial2.write(percentageByte);
}

void computeTemps(){
  for (int i = 0; i < 64; i++){
    K = 1.00 / (invT0 + invBeta*(log (1/(adcMax / (float) SensorValues[i] - 1.00))));
    //K = 1.00 / (invT0 + invBeta*(log ( adcMax / (float) SensorValues[i] - 1.00))); // This is if the thermistor was the top leg of the dividor, not the bottom
    C = K - 273.15;                      // convert to Celsius
    F = ((9.0*C)/5.00) + 32.00;   // convert to Fahrenheit
    //t[0] = K; t[1] = C; t[2] = F;
    SensorTemps[i] = C;
  }
}

float newValue = 0.0;
void smoothTemps(){
  for (int i = 0; i < 64; i++){
    newValue = SensorTemps[i];
    if (i-1 >= 0){ 
      newValue += .7*SensorTemps[i-1]; // add n-1
      if (i-2 >= 0){newValue += .3*SensorTemps[i-2];} // add n-2
      else{newValue += .3*SensorTemps[i-1];} // n-2 has to use n-1 value
    } else{newValue += SensorTemps[i];} // both n-1 and n-2 have to use the original value
    if (i+1 < 64){ 
      newValue += .7*SensorTemps[i+1]; // add n+1
      if (i+2 < 64){newValue += .3*SensorTemps[i+2];} // add n+2
      else{newValue += .3*SensorTemps[i+1];} // n+2 has to use n+1 value
    } else{newValue += SensorTemps[i];} // both n+1 and n+2 have to use the original value
    temporarySensorTemps[i] = newValue/3.0;
  }
  for (int i = 0; i < 64; i++){
    SensorTemps[i]=temporarySensorTemps[i];  
  }
}

void printSensorData(){
  for(int i = 0; i < 109; i ++){
    //Serial.println(1024);
    Serial.println(65536);
    Serial.println(0);
  }
  for(int i = 0; i < 64; i ++){
    Serial.println(10*SensorValues[i]);
    //Serial.println(10*(SensorValues[i]+3000));
  }
  for(int i = 0; i < 109; i ++){
    Serial.println(0);
    //Serial.println(1024);
    Serial.println(65536);
  }
}

void printSensorDataTemps(){
  for(int i = 0; i < 109; i ++){
    //Serial.println(1024);
    Serial.println(35);
    Serial.println(25);
  }
  for(int i = 0; i < 64; i ++){
    Serial.println(SensorTemps[i]);
    //Serial.println(10*(SensorValues[i]+3000));
  }
  for(int i = 0; i < 109; i ++){
    Serial.println(25);
    //Serial.println(1024);
    Serial.println(35);
  }
}

void printSensorDataTempsExpanded(){
  for(int i = 0; i < 13; i ++){
    //Serial.println(1024);
    Serial.println(35);
    Serial.println(20);
  }
  sensorExpanderCounter = 3;
  SensorTempsPrint[0] = SensorTemps[0];
  SensorTempsPrint[1] = SensorTemps[0];
  SensorTempsPrint[2] = SensorTemps[0];
  for(int i = 0; i < 64; i ++){
    if(i<63){
      SensorTempsPrint[sensorExpanderCounter] = SensorTemps[i];
      SensorTempsPrint[sensorExpanderCounter+1] = (6.0/7.0)*SensorTemps[i]+(1.0/7.0)*SensorTemps[i+1];
      SensorTempsPrint[sensorExpanderCounter+2] = (5.0/7.0)*SensorTemps[i]+(2.0/7.0)*SensorTemps[i+1];
      SensorTempsPrint[sensorExpanderCounter+3] = (4.0/7.0)*SensorTemps[i]+(3.0/7.0)*SensorTemps[i+1];
      SensorTempsPrint[sensorExpanderCounter+4] = (3.0/7.0)*SensorTemps[i]+(4.0/7.0)*SensorTemps[i+1];
      SensorTempsPrint[sensorExpanderCounter+5] = (2.0/7.0)*SensorTemps[i]+(5.0/7.0)*SensorTemps[i+1];
      SensorTempsPrint[sensorExpanderCounter+6] = (1.0/7.0)*SensorTemps[i]+(6.0/7.0)*SensorTemps[i+1];
      sensorExpanderCounter += 7;
    }
    else{
      SensorTempsPrint[sensorExpanderCounter] = SensorTemps[i];
      SensorTempsPrint[sensorExpanderCounter+1] = SensorTemps[i];
      SensorTempsPrint[sensorExpanderCounter+2] = SensorTemps[i];
      SensorTempsPrint[sensorExpanderCounter+3] = SensorTemps[i];
      sensorExpanderCounter+= 4;
    }
  }
  for(int i = 0; i < 448; i ++){
    Serial.println(SensorTempsPrint[i]);
  }
  for(int i = 0; i < 13; i ++){
    Serial.println(20);
    //Serial.println(1024);
    Serial.println(35);
  }
}

// allows a single function call for setting a mux channel. Note: this function currently
// only supports a single group of muxes (all using the same select lines).
// Note there are OOP libraries for analog muxes but this code is really simple.
// channel input is 0-7
void setMux(int channel){
  // if we are worried about speed, these digital writes need to be replaced. (and from hardware, pin selects
  // need to end up on one port!)
  if (channel%2 == 1){digitalWrite(pinANALOGSELECT0, HIGH);} else{digitalWrite(pinANALOGSELECT0, LOW);}
  if ((channel/2)%2 == 1){digitalWrite(pinANALOGSELECT1, HIGH);} else{digitalWrite(pinANALOGSELECT1, LOW);}
  if ((channel/4)%2 == 1){digitalWrite(pinANALOGSELECT2, HIGH);} else{digitalWrite(pinANALOGSELECT2, LOW);}
  // This hardware is only for an 8 to 1 mux
  //if ((channel/8)%2 == 1){digitalWrite(pinANALOGSELECT3, HIGH);} else{digitalWrite(pinANALOGSELECT3, LOW);}
  //delayMicroseconds(100); // give settling/ capacitor charge shuffle equilization time.
  delay(2);
}

// this function grabs the current values for each pin on the micro for the current mux setting, then figures out
// if anything interesting is happening ie data recieved and what kind of data and valid packets, etc.
void sampleSensors(int channel){
  for (int counter = 0; counter < 8; counter ++){
    sensorCounter = channel + counter*8;
    SensorValues[sensorCounter] = adc->analogRead(pinsSENSORS[counter]);
    //SensorValues[sensorCounter] = analogRead(pinsSENSORS[counter]);
  }
}

void fixSensorOrder(){
  for (int counter = 0; counter < 16; counter ++){
    temporarySensorValues[(15-counter)*2] = SensorValues[counter];
  }
  for (int counter = 16; counter < 32; counter ++){
    temporarySensorValues[1+((31-counter)*2)] = SensorValues[counter];
  }
  for (int counter = 32; counter < 48; counter ++){
    temporarySensorValues[33+((counter-32)*2)] = SensorValues[counter];
  }
  for (int counter = 48; counter < 64; counter ++){
    temporarySensorValues[32+((counter-48)*2)] = SensorValues[counter];
  }
  for (int counter = 0; counter < 64; counter ++){
    SensorValues[counter] = temporarySensorValues[counter];
  }
}

// setting up both adcs to run reasonably fast
void configureADCs(){
  // Sampling speed is how long the ADC waits for the internal sample and hold cap to equilize. 
  // For a large, low impedance external cap (.1u or larger), this can be very small (very fast).
  // Conversion Speed is what base speed is used for the ADC clock. It influences sampling time 
  // (which is set in number of adc clocks), as well as how fast the conversion clock runs.
  // Resolution *MAY* influence how many adc clocks are needed to make a conversion. 
  // Averaging is how many samples to take and average.
  adc->setReference(ADC_REFERENCE::REF_3V3, ADC_0); // options are: ADC_REFERENCE::REF_3V3, ADC_REFERENCE::REF_EXT, or ADC_REFERENCE::REF_1V2.
  adc->setSamplingSpeed(ADC_SAMPLING_SPEED::MED_SPEED); // options are VERY_LOW_SPEED, LOW_SPEED, MED_SPEED, HIGH_SPEED or VERY_HIGH_SPEED.
  adc->setConversionSpeed(ADC_CONVERSION_SPEED::MED_SPEED); // options are VERY_LOW_SPEED, LOW_SPEED, MED_SPEED, HIGH_SPEED_16BITS, HIGH_SPEED or VERY_HIGH_SPEED. VERY_HIGH_SPEED may be out of specs
  adc->setAveraging(8); // set number of averages
  adc->setResolution(16); // set number of bits

  adc->setReference(ADC_REFERENCE::REF_3V3, ADC_1);
  adc->setSamplingSpeed(ADC_SAMPLING_SPEED::MED_SPEED, ADC_1);
  adc->setConversionSpeed(ADC_CONVERSION_SPEED::MED_SPEED, ADC_1);
  adc->setAveraging(8, ADC_1); 
  adc->setResolution(16, ADC_1);  
}

void CalibrateADCs(){
  for(int i = 0; i < 8; i ++){
    setMux(i);
    sampleSensors(i);
  }
  for(int i=0; i<64; i++){
    SensorCalibrations[i] = SensorValues[i];
    //Serial.println(SensorCalibrations[i]);
  }
  //delay(5000);
  
}

void applyCalibrations(){
  for (int counter = 0; counter < 64; counter ++){
    SensorValues[counter] = SensorCalibrations[counter]-SensorValues[counter];
  }
}
