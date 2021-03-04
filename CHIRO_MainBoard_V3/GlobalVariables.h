


#ifndef GlobalVariables_h
#define GlobalVariables_h

#include <Arduino.h>
#include "Buttons.h"

// configuration information
float fltAmpGain = 3333.3; // common values: 1,000 for Teensy board, 10,000 for ATMEGA, 3,333.3 for ATMEGA reconfigured for 5 thermocouples
float fltThermocoupleCoefficient = 200; // in uV/degree C, takes into acount how many thermocouples there are.
// Type T is about 40uV/C
// Type E is about 60uV/C
// FSR is +/-12 ticks for 3535, and +/- 32 ticks for 0603
// for ATMEGA (12 bit nominal ADC), 3,333 gain and 200uV/C (x5 Type T), LSB is .366uV, or .0018C (further averaging can obtain higher res)
//float needleSensitivities[] = {0.1,0.2,0.35,0.65,1.25,2.3,4.5,8.0}; // map intSensitivity (range 1-8, adjusted to 0-7) to scalar values for temperature
  // ranges to plot on the needle graph. Units are in uV/lsb, or uV per LED step to take. Note this is the value for full steps
  // (ie, a value of 2 on the graph). If half-stepping is used, they will be divided in half in place.
// new mapping is in C/lsb, so variable thermocouples can be used interchangably. Roughly 1.87 gain per stage
//float needleSensitivities[] = {0.002,0.0034,0.0065,0.012,0.022,0.042,0.078,0.15};
float needleSensitivities[] = {0.15,0.078,0.042,0.022,0.012,0.0065,0.0034,0.002};
bool booPlotTemps = false;
bool booPlotTiming = false;

// PIN DECLARATIONS
int pinSTAYON = A5;
int pinANALOGDIFFAMP = A0;
int pinHEARTBEAT = 9;
int pinAUDIOLED = 6;
int pinBUZZER = 5;
int pinBATTERYVSENSE = A7;
int pinBOARDTEMP = A6;
int pinBUTTONOFF = 7;
int pinBUTTONBRIGHTNESSUP = 3;
int pinBUTTONBRIGHTNESSDOWN = 4;
int pinBUTTONSENSITIVITYUP = 8;
int pinBUTTONSENSITIVITYDOWN = A2;
int pinCS_LEDs = 2;

// VARIABLE DECLARATIONS
long lngBatSense = 0;
float fltBatSense = 0;
bool booLowBatLEDState = false;
bool booLowBatIndicator = false;

// timing and housekeeping
unsigned long lngHeartbeatTimer = 0;
unsigned long lngShutOffTimer = 0;
unsigned long lngLoopTimerStart = 0;
unsigned long lngLoopTimerEnd = 0;
unsigned long LNGloopTimer[50];
unsigned long lngBarGraphUpdateTimer = 0;
int intLoopTimerPointer = 0;
boolean booHeartbeatState = false;

int intSensitivity = 0;
int intBrightness = 0;
bool booModeAudio = 0;

// temperature/ADC tracking variable
int numSamples = 64; // num samples is how many ADC reads get converted into a single average number. Should be at least 4 and divisible by 4, since we up-rez the 10 bit ADC to 12 bit.
long lngAmpRaw = 0;
float fltTempDiffRawV = 0;
float fltTempDiffuV = 0;
#define NUMAVERAGES 10 // slightly different than numSamples, this is how many historical sample results are saved and averaged when printing information. It forms a rolling window time domain filter.
float FLTtempDiffHist[NUMAVERAGES];
int intTempDiffHistPointer = 0;
float fltTempDiffAveuV = 0;
float fltTempDiffAveC = 0;
float fltADCDiffOffset = 0;

// button variables
bool booCheckButtonHeld = false;
ezButton btnOff(pinBUTTONOFF);
ezButton btnBrightnessUp(pinBUTTONBRIGHTNESSUP);
ezButton btnBrightnessDown(pinBUTTONBRIGHTNESSDOWN);
ezButton btnSensitivityUp(pinBUTTONSENSITIVITYUP);
ezButton btnSensitivityDown(pinBUTTONSENSITIVITYDOWN);

// fancy variables for LEDs
SPISettings mySetting(1000,MSBFIRST,SPI_MODE0);
MCP mcpLEDs0  (0, pinCS_LEDs);
MCP mcpLEDs1  (1, pinCS_LEDs);
MCP mcpLEDs2  (2, pinCS_LEDs);
int intNeedlePosition = 0; 
int intBarGraphValue = 0;
int intHighSideDrive = 0;
int intHighSideDrive2 = 0;
int intLowSideDrive = 0;
int intLowSideDrive2 = 0;
int mask = 0;
int intMCP0 = 0;
int intMCP1 = 0;
int intMCP2 = 0;


#endif
