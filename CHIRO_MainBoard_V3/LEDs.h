

#ifndef LEDs_h
#define LEDs_h

#include <Arduino.h>
#include "GlobalVariables.h"
#include <SPI.h>
#include <MCP23S17.h>

const uint8_t PROGMEM positiveHighSideDrives[] = {17,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,6,6,6,6,7,7,7,7,8,8,8,8};
const uint8_t PROGMEM negativeHighSideDrives[] = {17,9,9,9,9,10,10,10,10,11,11,11,11,12,12,12,12,13,13,13,13,14,14,14,14,15,15,15,15,16,16,16,16};
const uint8_t PROGMEM positiveLowSideDrives[] = {1,1,2,3,4,4,3,2,1,1,2,3,4,4,3,2,1,1,2,3,4,4,3,2,1,1,2,3,4,4,3,2,1};
const uint8_t PROGMEM negativeLowSideDrives[] = {1,1,2,3,4,4,3,2,1,1,2,3,4,4,3,2,1,1,2,3,4,4,3,2,1,1,2,3,4,4,3,2,1}; // are these always identical or
// just with my mapping? we could possibly simnplify the lookup logic a bit.

// map a position from +/- 32 to actual drive signals for LEDs to draw a needle
// gauge. needlePosition has a range of double the number of LEDs (so with a gauge
// made up of 65 LEDs, 32+/32- with a zero position LED, use +/-64 for needlePosition
// variable. Use only even numbers to plot single LEDs, only odd numbers to plot only
// double LEDs, or both to get half stepping.
// also map a bar graph value from 0-8 to discrete LEDs. One function for both because
// they share port expanders.
void updateLEDDrivers(){
  // NEEDLE GAUGE
  // first, figure out what high/low drives need to be on based on the strange mapping, using lookuptables.
  // NOTE THAT CURRENTLY MAX RANGE IS HARDCODED, SO FOR A NEEDLE GAUGE WITH MORE/LESS LEDS THIS WOULD NOT WORK.
  // SOMEDAY, PARAMETERIZE THIS.
  //Serial.println(intNeedlePosition);
  if (intNeedlePosition > 64){intNeedlePosition = 64;}
  if (intNeedlePosition < -64){intNeedlePosition = -64;}
  if (abs(intNeedlePosition)%2==1){ // value is odd, plot BOTH above and below target value
    if (intNeedlePosition > 0){
      intHighSideDrive = pgm_read_byte(&positiveHighSideDrives[(intNeedlePosition-1)/2]);
      intHighSideDrive2 = pgm_read_byte(&positiveHighSideDrives[(intNeedlePosition+1)/2]); 
      intLowSideDrive = pgm_read_byte(&positiveLowSideDrives[(intNeedlePosition-1)/2]);
      intLowSideDrive2 = pgm_read_byte(&positiveLowSideDrives[(intNeedlePosition-1)/2]);     
    } else{
      intHighSideDrive = pgm_read_byte(&negativeHighSideDrives[(abs(intNeedlePosition)-1)/2]);
      intHighSideDrive2 = pgm_read_byte(&negativeHighSideDrives[(abs(intNeedlePosition)+1)/2]); 
      intLowSideDrive = pgm_read_byte(&negativeLowSideDrives[(abs(intNeedlePosition)-1)/2]);
      intLowSideDrive2 = pgm_read_byte(&negativeLowSideDrives[(abs(intNeedlePosition)-1)/2]);        
    }
  }else{ // value is even, plot only a single value
    if (intNeedlePosition > 0){
      intHighSideDrive = pgm_read_byte(&positiveHighSideDrives[(intNeedlePosition)/2]); intHighSideDrive2 = intHighSideDrive;
      intLowSideDrive = pgm_read_byte(&positiveLowSideDrives[(intNeedlePosition)/2]); intLowSideDrive2 = intLowSideDrive;
    } else{
      intHighSideDrive = pgm_read_byte(&negativeHighSideDrives[abs(intNeedlePosition)/2]); intHighSideDrive2 = intHighSideDrive;
      intLowSideDrive = pgm_read_byte(&negativeLowSideDrives[abs(intNeedlePosition)/2]); intLowSideDrive2 = intLowSideDrive;
    }
  }

  // now, map high/low drives to port expander pins to drive, taking into account brightness settings
  // do this without changing other things that may be on the port expanders
  intMCP0 = 0b1111111111111111; // all inputs, don't drive any LEDs
  intMCP1 = 0b1111111111111111;
  intMCP2 = 0b1111111111111111;
  // high side drives
  if (intHighSideDrive !=17){ mask = 0b0000000000000001 << (intHighSideDrive-1); intMCP0 = intMCP0 & ~mask;}
  else{intMCP2 = intMCP2 & 0b1110111111111111;}
  if (intHighSideDrive2 !=17){ mask = 0b0000000000000001 << (intHighSideDrive2-1); intMCP0 = intMCP0 & ~mask;}
  else{intMCP2 = intMCP2 & 0b1110111111111111;}
  // low side drives
  mask = 0b0000000000000001 << ((intLowSideDrive-1)*4);
  mask = mask << (4-intBrightness); 
  intMCP1 = intMCP1 & ~mask;
  mask = 0b0000000000000001 << ((intLowSideDrive2-1)*4);
  mask = mask << (4-intBrightness);
  intMCP1 = intMCP1 & ~mask;

  // BAR GRAGH
  if (millis() - lngBarGraphUpdateTimer < 2500){
    // set low drive for brightness
    mask = 0b0000000000000001 << (4-intBrightness);
    intMCP2 = intMCP2 & ~mask;
    if (intBarGraphValue < 1){intBarGraphValue = 1;} if (intBarGraphValue > 8){intBarGraphValue = 8;}
    mask = 0b0000000000000000;
    //mask = 0b0000000011111111 >> (8-intBarGraphValue); // this is clever, it would have worked with proper layout
    if (intBarGraphValue==1){mask=0b0000000010000000;}
    if (intBarGraphValue==2){mask=0b0000000011000000;}
    if (intBarGraphValue==3){mask=0b0000000011100000;}
    if (intBarGraphValue==4){mask=0b0000000011110000;}
    if (intBarGraphValue==5){mask=0b0000000011111000;}
    if (intBarGraphValue==6){mask=0b0000000011111001;}
    if (intBarGraphValue==7){mask=0b0000000011111011;}
    if (intBarGraphValue==8){mask=0b0000000011111111;}
    mask = mask << 4;
    intMCP2 = intMCP2 & ~mask;
  }

  //Serial.print("Target LED drives: MCP0 (high sides): "); Serial.print(intMCP0,BIN);Serial.print(", MCP1 (low sides): ");Serial.print(intMCP1,BIN);Serial.print(", MCP2 (Bar graph/ mixed): ");Serial.println(intMCP2,BIN);

  // SET DRIVERS
  mcpLEDs0.pinMode(intMCP0);     // Use word-write mode to set all of the pins on inputchip to be inputs
  mcpLEDs1.pinMode(intMCP1);
  mcpLEDs2.pinMode(intMCP2);
}

// set up all the port expanders for beginning their journey, to be called once.
void initializePortExpanders(){  
  SPI.begin();
  SPI.beginTransaction(mySetting);
  mcpLEDs0.begin();
  mcpLEDs1.begin();
  mcpLEDs2.begin();
  SPI.beginTransaction(mySetting);  
  mcpLEDs0.digitalWrite(0xFFFF); // high side drives
  mcpLEDs1.digitalWrite(0x0000); // low side drives
  mcpLEDs2.digitalWrite(0b1111111111110000); // mixed drive
  mcpLEDs0.pinMode(0xFFFF);     // Use word-write mode to set all of the pins on inputchip to be inputs
  mcpLEDs1.pinMode(0xFFFF);
  mcpLEDs2.pinMode(0xFFFF);
  mcpLEDs0.pullupMode(0x0000);  // Use word-write mode to Turn off the internal pull-up resistors.
  mcpLEDs1.pullupMode(0x0000);
  mcpLEDs2.pullupMode(0x0000);
}

#endif
