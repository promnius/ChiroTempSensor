

#ifndef GlobalVariables_h
#define GlobalVariables_h

#include <Arduino.h>
#include "Buttons.h"
#include <ADC.h>
#include "Protocentral_ADS1220.h"

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
int intDedIntensity = 0;
float FLTtempDiffHist[6];
int intTempDiffHistPointer = 0;
float fltTempDiffAve = 0;
long intLedPosition = 0;
int intLedIntensity = 0;
int intMaxBrightness = 160;

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
#define RED_CHANNEL 2
#define GREEN_CHANNEL 1
#define BLUE_CHANNEL 0

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

const uint8_t PROGMEM gamma8[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

#endif
