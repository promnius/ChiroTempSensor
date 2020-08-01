
#include <WS2812Serial.h> 

// PIN DECLARATIONS
int pinSTAYON = 3;
int pinANALOGDIFFAMP = A6;
int pinANALOGAMP0 = A1;
int pinANALOGAMP1 = A2;
int pinHEARTBEAT = 4;
int pinLEDDATA = 1;
int pinPANELDRIVE0 = 9;
int pinPANELDRIVE1 = 10;

// VARIABLE DECLARATIONS
unsigned long lngHeartbeatTimer = 0;
unsigned long lngShutOffTimer = 0;
boolean booHeartbeatState = false;

// fancy variables for LEDs
const int numled = 24;
byte drawingMemory[numled*3];         //  3 bytes per LED
DMAMEM byte displayMemory[numled*12]; // 12 bytes per LED
WS2812Serial leds(numled, displayMemory, drawingMemory, pinLEDDATA, WS2812_GRB);
#define RED    0xFF0000
#define GREEN  0x00FF00
#define BLUE   0x0000FF
#define YELLOW 0xFFFF00
#define PINK   0xFF1088
#define ORANGE 0xE05800
//#define WHITE  0xFFFFFF

void setup() {
  pinMode(pinSTAYON, OUTPUT);
  digitalWrite(pinSTAYON, HIGH);
  pinMode(pinHEARTBEAT, OUTPUT);
  pinMode(pinPANELDRIVE0, OUTPUT);
  pinMode(pinPANELDRIVE1, OUTPUT);
  lngHeartbeatTimer = millis();
  lngShutOffTimer = millis();
  //leds.begin();
  //leds.setPixel(2, GREEN);
  //leds.show();
  //analogWriteResolution(12);
  analogWriteResolution(8);
  analogWrite(A14, 2048);

  Serial.begin(1000000);
  Serial3.begin(9600);

  //digitalWrite(pinPANELDRIVE1, LOW);
  //digitalWrite(pinPANELDRIVE0, HIGH);
}

int intPercentNeedleGauge = 0;

void loop() {
  if (millis() - lngHeartbeatTimer > 1000){
    booHeartbeatState = !booHeartbeatState;
    digitalWrite(pinHEARTBEAT, booHeartbeatState);
    lngHeartbeatTimer = millis();
  }

  while (Serial3.available() > 0) {
    intPercentNeedleGauge = Serial3.read();
    Serial.println(intPercentNeedleGauge);
    analogWrite(pinPANELDRIVE0, intPercentNeedleGauge);
    analogWrite(pinPANELDRIVE1, 255-intPercentNeedleGauge);
  }

  //intPercentNeedleGauge = 4*(508-analogRead(pinANALOGDIFFAMP));
  //analogWrite(pinPANELDRIVE0, 2048 + intPercentNeedleGauge*20);
  //analogWrite(pinPANELDRIVE1, 2048 - intPercentNeedleGauge*20);
  delay(30);
}
