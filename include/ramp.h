#ifndef RAMPS_H
#define RAMPS_H
#include <SPI.h>
#include <stdint.h>
#include "utils.h"
#include "ad5791.h"
#include "ad4115.h"
#include <cstdlib>
using namespace std;

class RAMPS
{
protected:


private:
	double calcDv(uint8_t channelsDAC[4], double vi[4], double vf[4], double nSteps);
	uint8_t setVi(uint8_t channelsDAC[4], double vi[4]);
	double roundToSixDecimalPlaces(double value);
	double dv[4] = {0, 0, 0, 0};
    AD5791& dac;
    AD4115& adc;
 	int mValue;


public:
	uint8_t rampCmd[20];
	uint8_t simpleRamp(uint8_t channelsDAC[4], double vi[4], double vf[4], double nSteps, double del);
	uint8_t bufferRamp(uint8_t channelsDAC[4], double vi[4], double vf[4], uint32_t nSteps, uint32_t del, uint8_t channelsADC[16]);
	uint8_t simpleRampIteration(uint8_t channelsDAC[4], double vi[4], double nSteps, double del);
	uint8_t bufferRampIteration(uint8_t channelsDAC[4], double vi[4], uint32_t nSteps, uint32_t del, uint8_t channelsADC[16]);

	// Constructor
  	RAMPS(AD5791& dac, AD4115& adc);
};


#endif // RAMPS_H