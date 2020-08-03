
#include "Buttons.h"

ezButton::ezButton(int pin) {
	btnPin = pin;
	lngDebounceTime = 20;
	lngCount = 0;
	intCountMode = COUNT_PRESSING;
	booHasReadEdge = true;

  intInversion = LOW_PRESSED;

	lngPreviousStateTime = 0;
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
	return(lngPreviousStateTime);
}

int ezButton::getStateRaw(void) {
	return digitalRead(btnPin);
}

void ezButton::setLogicInversion(int invert) {
  intInversion = invert;
}

bool ezButton::getPressingEdge(void) {
	if(intState == HIGH && booHasReadEdge == false){
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

void ezButton::update(void) {
	intRawState = digitalRead(btnPin);
  if (intInversion == LOW_PRESSED){intRawState = !intRawState;}

	// check to see if you just pressed the button
	// (i.e. the input went from LOW to HIGH), and you've waited long enough
	// since the last press to ignore any noise:

	// If the switch/button changed, and the time has been long enough
	if (intRawState != intState && (millis() - lngLastTransistionTime)>lngDebounceTime) {
		lngPreviousStateTime = millis() - lngLastTransistionTime;
		lngLastTransistionTime = millis();
		booHasReadEdge = false;
		intState = intRawState;
		if(intCountMode == COUNT_BOTH){lngCount++;}
		else if(intCountMode == COUNT_PRESSING && intState == 1){lngCount++;} // intState==1 means we just pressed
		else if(intCountMode == COUNT_RELEASING && intState == 0){lngCount++;}
	}
}
