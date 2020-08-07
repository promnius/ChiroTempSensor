
// this library implements buttons with timing for press and hold
// there may be a minor bug where an extra button press is picked up
// around changing the inversion mode

#include "Buttons.h"

ezButton::ezButton(int pin) {
  intPreviousStatePointer = 0;
	btnPin = pin;
	lngDebounceTime = 40;
	lngCount = 0;
	intCountMode = COUNT_PRESSING;
	booHasReadEdge = true;
  lngDoubleTapGapThreshold = 150;

  intInversion = LOW_PRESSED;

  for (int i = 0; i < 6; i ++){LNGpreviousStateTime[i] = 0;}
  
	lngLastTransistionTime = millis();

	pinMode(btnPin, INPUT_PULLUP);

	intState = digitalRead(btnPin);
	intRawState = intState;
}

void ezButton::setDebounceTime(unsigned long time) {
	lngDebounceTime = time;
}

int ezButton::getState(void) {
	return intState;
}

unsigned long ezButton::getStateTime(void) {
	return (millis() - lngLastTransistionTime);
}

unsigned long ezButton::getLastStateTime(void) {
	return(LNGpreviousStateTime[intPreviousStatePointer]);
}

int ezButton::getStateRaw(void) {
	return digitalRead(btnPin);
}

void ezButton::setLogicInversion(int invert) {
  intInversion = invert;
}

bool ezButton::getPressingEdge(void) {
	if(intState == HIGH && booHasReadEdge == false){
    //Serial.print("Button Timer Buffer: ");
    //Serial.print(LNGpreviousStateTime[0]);Serial.print(',');
    //Serial.print(LNGpreviousStateTime[1]);Serial.print(',');
    //Serial.print(LNGpreviousStateTime[2]);Serial.print(',');
    //Serial.print(LNGpreviousStateTime[3]);Serial.print(',');
    //Serial.print(LNGpreviousStateTime[4]);Serial.print(',');
    //Serial.print(LNGpreviousStateTime[5]);Serial.print(". Pointer: "); 
    //Serial.println(intPreviousStatePointer);
		booHasReadEdge = true;
		return true;
	}else{return false;}
}

bool ezButton::getReleasingEdge(void) {
	if(intState == LOW && booHasReadEdge == false) {
		booHasReadEdge = true;
		return true;
	}else{return false;}
}

void ezButton::setCountMode(int mode) {
	intCountMode = mode;
}

unsigned long ezButton::getCount(void) {
	return lngCount;
}

void ezButton::resetCount(void) {
	lngCount = 0;
}

// theres a much cleaner way to do this with math!
// the assumption is the offsets will be small.
// please make this better.
int ezButton::getArrayPointer(int pointerOffset){ 
  int temp = 0;
  temp = intPreviousStatePointer + pointerOffset;
  if (temp < 0){temp += 6*(temp%6);}
  else {temp -= 6*(temp%6);}
  //Serial.print("New Pointer Position: "); Serial.println(temp);
  return temp;
}

bool ezButton::getTripleTap(void){
  if (getDoubleTap() && LNGpreviousStateTime[getArrayPointer(-2)] < lngDoubleTapGapThreshold){return true;}
  else {return false;}
}

bool ezButton::getDoubleTap(void){
  if (LNGpreviousStateTime[intPreviousStatePointer] < lngDoubleTapGapThreshold){return true;}
  else {return false;}
}

void ezButton::setDoupleTapThreshold(int thresh){
  lngDoubleTapGapThreshold = thresh;
}

void ezButton::update(void) {
	intRawState = digitalRead(btnPin);
  if (intInversion == LOW_PRESSED){intRawState = !intRawState;}

	// check to see if you just pressed the button
	// (i.e. the input went from LOW to HIGH), and you've waited long enough
	// since the last press to ignore any noise:

	// If the switch/button changed, and the time has been long enough
	if (intRawState != intState && (millis() - lngLastTransistionTime)>lngDebounceTime) {
    intPreviousStatePointer ++; if (intPreviousStatePointer > 5){intPreviousStatePointer = 0;}
		LNGpreviousStateTime[intPreviousStatePointer] = millis() - lngLastTransistionTime;
		lngLastTransistionTime = millis();
		booHasReadEdge = false;
		intState = intRawState;
		if(intCountMode == COUNT_BOTH){lngCount++;}
		else if(intCountMode == COUNT_PRESSING && intState == 1){lngCount++;} // intState==1 means we just pressed
		else if(intCountMode == COUNT_RELEASING && intState == 0){lngCount++;}
	}
}
