
// todo: 
// implement LED bar graph and mode changes
// test buzzer/ add to specific modes/ add audio light
// add power-on voltage bar graph animation
// finish fixing ads1220 library, move it to this project
// Comments
// fix minor bug with button library on changing inversion status

// WARNING ABOUT THE ADS1220 LIBRARY:
// I have modified the library, as of the writing of this code the library had a bad bug that
// prevented useful reads from occuring.

#include <WS2812Serial.h> 
#include "Protocentral_ADS1220.h"
#include "Buttons.h"
#include <ADC.h>

// PIN DECLARATIONS
int pinSTAYON = 3;
int pinANALOGDIFFAMP = 20;
int pinHEARTBEAT = 0;
int pinAUDIOLED = 4;
int pinBUZZER = 5;
int pinLEDDATA = 1;
int pinADS1220CS = 8;
int pinADS1220DRDY = 7;
int pinMODEBUTTON = 2;
int pinBATTERYVSENSE = 14;
int pinBOARDTEMP = 19;

// VARIABLE DECLARATIONS
unsigned long lngHeartbeatTimer = 0;
unsigned long lngShutOffTimer = 0;
boolean booHeartbeatState = false;
int intMode = 0;
int intADCResolution = 12;
float fltThermocouple1Voltage = 0;
float fltThermocouple2Voltage = 0;
long lngBatSense = 0;
float fltBatSense = 0;
bool booModeAudio = 0;
bool booCheckButtonHeld = false;
float fltTempDiff = 0;

ezButton modeButton(pinMODEBUTTON);
ADC *adc = new ADC();

// fancy variables for LEDs
const int numled = 27;
byte drawingMemory[numled*3];         //  3 bytes per LED
DMAMEM byte displayMemory[numled*12]; // 12 bytes per LED
WS2812Serial leds(numled, displayMemory, drawingMemory, pinLEDDATA, WS2812_GRB);
/*
#define RED    0xFF0000
#define GREEN  0x00FF00
#define BLUE   0x0000FF
#define YELLOW 0xFFFF00
#define PINK   0xFF1088
#define ORANGE 0xE05800
#define WHITE  0xFFFFFF
*/
#define RED    0x160000
#define GREEN  0x001600
#define BLUE   0x000016
#define YELLOW 0x101400
#define PINK   0x120009
#define ORANGE 0x100400
#define WHITE  0x101010

// ADC library is poorly documented, here are the useful functions for this project
// set_data_rate(DR_20SPS) sets sample rate, can be [20,45,90,175,330,600,1000]
// set_pga_gain(PGA_GAIN_1) sets gain, can be [1,2,4,8,16,32,64,128]
// Read_SingleShot_WaitForData() is a blocking function that takes a single sample in single shot mode (after modifying the library)
// Start_Conv() is not usually needed- it is already called within the read functions, but this would start a sample manually
// set_conv_mode_continuous() or single_shot
// select_mux_channels(MUX_AIN0_AIN1) many options are available, the other one we need here is MUX_AIN2_AIN3
// PGA_ON() this is set by default anyways
// Single_shot_mode_ON() !! this only exists in the h file, not the c file, so I think it is obsolete, given that the set_conv_mode_single_shot also exists and is implemented.

// ADC CONSTANTS AND VARIABLES:
#define PGA          128              // Programmable Gain
// NOTE THIS ONLY SETS THE MATH, NOT THE ACTUAL GAIN, as the gain is set using predefined numbers, as it
// is set with a 3 bit register. they must both be set in unison or the results will be wrong.
#define VREF_ADC         2.048            // Internal reference of 2.048V
#define VFSR         VREF_ADC/PGA
#define FULL_SCALE   (((long int)1<<23)-1)
Protocentral_ADS1220 externalADC;
int32_t adc_data1 = 0;
int32_t adc_data2 = 0;

void setup() {
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
  //analogWrite(pinBUZZER, 128);

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
  leds.setPixel(2, GREEN);
  leds.show();
  
  
  Serial.begin(1000000);

  delay(500);
  lngBatSense = adc->analogRead(pinBATTERYVSENSE);
  fltBatSense = 3.3*2*((float)lngBatSense)/((float)pow(2,intADCResolution)); // 3.3 is Vref, 2 is resistor divider scalar
  Serial.print("Battery Voltage: "); Serial.print(fltBatSense); Serial.println(" Volts.");
  delay(500);
  Serial.println("Setup Finished!");
}

void loop() {
  if (millis() - lngHeartbeatTimer > 1000){
    booHeartbeatState = !booHeartbeatState;
    digitalWrite(pinHEARTBEAT, booHeartbeatState);
    lngHeartbeatTimer = millis();
  }

  if (millis() - lngShutOffTimer > 300000){ // 5 minutes
    digitalWrite(pinSTAYON, LOW); // shut off, not reversible without human interaction
  }

  //Serial.println(analogRead(pinANALOGDIFFAMP)); // to use the old analog amp input
  externalADC.select_mux_channels(MUX_AIN0_AIN1);
  delay(2);
  adc_data1 = externalADC.Read_SingleShot_WaitForData();
  fltThermocouple1Voltage = convertADCToMicroV(adc_data1);
  externalADC.select_mux_channels(MUX_AIN2_AIN3);
  delay(2);
  adc_data2 = externalADC.Read_SingleShot_WaitForData();
  fltThermocouple2Voltage = convertADCToMicroV(adc_data2);

  fltTempDiff = fltThermocouple1Voltage + fltThermocouple2Voltage; // addition because one is negative

  Serial.print("Thermocouple Voltages: "); Serial.print(fltThermocouple1Voltage);Serial.print(", ");
  Serial.print(fltThermocouple2Voltage); Serial.println();

  //delay(300); // not needed with the sigma delta adc- it takes long enough to perform conversions!

  // ADD CODE FOR LED BAR GRAPH OUTPUT, OTHER LEDS, AND MODE BUTTON INTERACTION
  modeButton.update();
  if (modeButton.getPressingEdge()==true){ // button pressed!
    intMode += 1;
    if (intMode > 7){intMode = 0;}
    Serial.println("Mode Button Pressed!");
    booCheckButtonHeld = true;
  }
  if (modeButton.getStateTime() > 4000 && booCheckButtonHeld == true){
    booCheckButtonHeld = false;
    booModeAudio = !booModeAudio;
    Serial.println("Audio State Changed!");
  }

  updateLEDs();
}

float convertADCToMicroV(int32_t i32data)
{
    return (float)((i32data*VFSR*1000000)/FULL_SCALE);
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

void updateLEDs(){
  long ledPosition;
  leds.clear();
  if (intMode == 0){
    ledPosition = (int)fltTempDiff;
    if (ledPosition > 12){
      ledPosition = 12;
    } else if (ledPosition < -12){
      ledPosition = -12;
    }
    ledPosition += 12;
    leds.setPixel(ledPosition, GREEN);
  }
  leds.show();
}
