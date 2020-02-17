#include "DigitalFilters.h"
#include "Streaming.h"
#include "ADC_Sampler.h"
#include "TimeOut.h"

Stream &s = Serial; // makke reference of output comunication

const uint8_t ADC_sequencer_size = 11;
#define LEFT 0
#define RIGHT 1
#define SUB 2

int outputBuffer[256][3] = {0};
uint8_t outputBufferCountFront = 0;
uint8_t outputBufferCountRear = 0;

constexpr float dtUsed = 0.0000226757369614512471655328;  //   1/44100Hz
int Fc = 100;
LowPassFilter lpf1(dtUsed,Fc * 2 * M_PI);

Interval monitor;

void setup() {
	Serial.begin(115200);
	monitor.interval(1000, printOutput);
	ADC_Sampler::begin(44100, 0,1,2,3);
	delay(5000);
	s << "L,R,S" << endl;
	ADC_Sampler::bufferReset(); //discard buffer to catchup
} 

int arrear = 0;

void loop() {
	signalTraitement();
	TimeOut::handler();
}

void signalTraitement(){
	if (ADC_Sampler::available()) {
		uint16_t* data = ADC_Sampler::data();
		int L = data[0] - data[1]; /// add positive and negative ADC
		int R = data[2] - data[3];
		int mono = (L+R)/2; /// Mae mono center channel
		outputBuffer[outputBufferCountFront][SUB] = lpf1.update(mono); //make sub with center
		outputBuffer[outputBufferCountFront][LEFT] = L - outputBuffer[outputBufferCountFront][SUB]; // remote sub of Left and Right channel
		outputBuffer[outputBufferCountFront][RIGHT] = R - outputBuffer[outputBufferCountFront][SUB];
		outputBufferCountFront++; // increment buffer
	}
	if (arrear < ADC_Sampler::arrearSize() ) arrear = ADC_Sampler::arrearSize();
}

bool outputAvailale() {
	if (  ( (outputBufferCountFront-1) - outputBufferCountRear) > 0 ) return true;
	return false;
}

void printOutput() {
	if ( outputAvailale() ) {
		s << outputBuffer[outputBufferCountRear][LEFT] << "," <<
		     outputBuffer[outputBufferCountRear][RIGHT] << "," <<
			 outputBuffer[outputBufferCountRear][SUB] << "," << endl <<
			 arrear << endl;
		outputBufferCountRear+= 10;
	}
}