

#ifndef Buttons_h
#define Buttons_h

#include <Arduino.h>

#define COUNT_PRESSING 0
#define COUNT_RELEASING  1
#define COUNT_BOTH    2
#define HIGH_PRESSED  0
#define LOW_PRESSED   1

class ezButton
{
	private:
		int btnPin;
		unsigned long lngDebounceTime;
		unsigned long lngCount;
		int intCountMode;
    int intInversion;
    unsigned long lngDoubleTapGapThreshold;

		int intRawState;
		int intState;  // the previous steady state from the input pin, used to detect pressed and released event
		bool booHasReadEdge;

		unsigned long LNGpreviousStateTime[6]; // the length of time the previous state existed for. we record a handful in case you
    // want to do advanced pattern detection, but they're not used right now.
    int intPreviousStatePointer;
		unsigned long lngLastTransistionTime; // the last time the output pin was toggled. NOTE this is not natively a duration like
			// lngPreviousStateTime, but rather a timestamp. 

	public:
		ezButton(int pin);
		void setDebounceTime(unsigned long time);
		void setLogicInversion(int invert);
		int getState(void); // what is the active state of the switch (with debounce). 1 means pressed.
		unsigned long getStateTime(void); // how long have we been in the current state? useful if you want something to happen after 
		// the user holds the button for a certain length of time.
		unsigned long getLastStateTime(void); // how long were we in the last state? useful if you want something to happen after 
		// a button was held for a certain time, but you want it to execute after the button is released. keeping track of this in
		// the button library reduces the amount of external code needed to make this happen.
		int getStateRaw(void); // just a digital read, probably not all that useful
		bool getPressingEdge(void); // similar to get state, but these will set an internal flag, so they only return true once
		bool getReleasingEdge(void); // per state, ie, you must find another edge before they will return true again.
		void setCountMode(int mode);
		unsigned long getCount(void);
		void resetCount(void);
		void update(void);
    bool getDoubleTap(void);
    void setDoupleTapThreshold(int thresh);
};

#endif
