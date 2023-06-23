#include "../include/ramp.h"
#include "../include/ad5791.h"
#include "../include/ad4115.h"
#include <stdint.h>
#include <SPI.h>
#include <cstdlib>
#include <WString.h>
#include <Arduino.h>
#include <string>
using namespace std;

/**
 * @brief Constructs a RAMPS object.
 *
 * This constructor initializes a RAMPS object with the specified AD5791 DAC and AD4115 ADC objects.
 *
 * @param dac The AD5791 DAC object.
 * @param adc The AD4115 ADC object.
 */
RAMPS::RAMPS(AD5791& dac, AD4115& adc) : dac(dac), adc(adc) {}

/**
 * @brief Sets the voltage levels for the specified channels on the RAMPS board.
 *
 * This function sets the voltage levels for the specified channels on the RAMPS board using the AD5791 DAC.
 * The voltage values are provided in the 'vi' array for each channel specified in the 'channelsDac' array.
 *
 * @param channelsDac An array indicating which channels to set the voltage for on the DAC.
 * @param vi An array of voltage values to set for each corresponding channel.
 * @return void
 */
uint8_t RAMPS::setVi(uint8_t channelsDac[4], double vi[4]) {
	double voltage = 0;
  for (int i = 0; i < 4; i ++) {
		if (channelsDac[i] == 1) {
			voltage = dac.setVoltage(i, vi[i], true);
      // Serial.println("DAC ");
		  // Serial.print(i);
      // Serial.print(" | SET INITIALLY TO ");
      // Serial.print(vi[i]);
      // Serial.println(voltage);
    }
	}
}

/**
 * @brief Calculates the voltage step size for each specified channel on the RAMPS board.
 *
 * This function calculates the voltage step size for each specified channel on the RAMPS board using the initial
 * and final voltage values provided in the 'vi' and 'vf' arrays, respectively. The number of steps is determined
 * by the 'nSteps' parameter. The calculated voltage step sizes are stored in the 'dv' array within the class.
 *
 * @param channelsDac An array indicating which channels to calculate the voltage step size for.
 * @param vi An array of initial voltage values for each corresponding channel.
 * @param vf An array of final voltage values for each corresponding channel.
 * @param nSteps The number of steps to divide the voltage range into.
 * @return void
 */
double RAMPS::calcDv(uint8_t channelsDac[4], double vi[4], double vf[4], double nSteps) {
  //Updates values of dv[i] stored in class:private.
	for (int i = 0; i < 4; i++) {
		if (channelsDac[i] == 1) {
      dv[i] = (vf[i] - vi[i]) / nSteps;			
		}
	}
  //Serial.println("");

	return 0;
}

/**
 * @brief Performs a simple ramp iteration for the specified channels on the RAMPS board.
 *
 * This function performs a simple ramp iteration for the specified channels on the RAMPS board. The ramp iteration
 * is controlled by the 'nSteps' parameter, which determines the number of steps in the ramp. The 'vi' array contains
 * the initial voltage values for each corresponding channel. The 'del' parameter specifies the delay in milliseconds
 * between each step of the ramp. The ramp iteration updates the voltage of each channel incrementally based on the
 * voltage step size previously calculated and stored in the 'dv' array. The updated voltage values are applied to
 * the DAC channels using the 'setVoltage' function. The LDAC pin is set to low to update all channels simultaneously
 * after each step of the ramp. The function includes optional Serial print statements for debugging or logging purposes.
 *
 * @param channelsDac An array indicating which channels to perform the ramp iteration on.
 * @param vi An array of initial voltage values for each corresponding channel.
 * @param nSteps The number of steps in the ramp iteration.
 * @param del The delay in milliseconds between each step of the ramp.
 * @return void
 */
uint8_t RAMPS::simpleRampIteration(uint8_t channelsDac[4], double vi[4], double nSteps, double del) {

  double dv_j;

  delay(del);
  
  for (int i = 0; i < nSteps; i++) {
    for (int j = 0; j < 4; j++) {
      if (channelsDac[j] == 1) {
        dv_j = dv[j];

        //Update channel i voltage
        dac.setVoltage(j, vi[j] + ((i+1)*dv_j), false);
        Serial.println(vi[j] + ((i+1)*dv_j));
        Serial.print("vReadings[j]");
        Serial.println(dac.vReadings[j]);
      }
    }
    //set LDAC pin to low to update all channels simultaneously
    dac.updateAnalogOutputs();
    
    //delay of input delay
    delay(del);
  }
  return 0;
}

/**
 * @brief Performs a buffer ramp iteration for the specified channels on the RAMPS board.
 *
 * This function performs a buffer ramp iteration for the specified channels on the RAMPS board. The ramp iteration
 * is controlled by the 'nSteps' parameter, which determines the number of steps in the ramp. The 'vi' array contains
 * the initial voltage values for each corresponding channel. The 'del' parameter specifies the delay in milliseconds
 * between each step of the ramp. The ramp iteration updates the voltage of each channel incrementally based on the
 * voltage step size previously calculated and stored in the 'dv' array. The updated voltage values are applied to
 * the DAC channels using the 'setVoltage' function. The LDAC pin is set to low to update all channels simultaneously
 * after each step of the ramp. The function includes calls to the 'bufferRampFullReading' function to read the ADC
 * values after each step of the ramp. Optional Serial print statements can be uncommented for debugging or logging
 * purposes.
 *
 * @param channelsDac An array indicating which channels to perform the ramp iteration on.
 * @param vi An array of initial voltage values for each corresponding channel.
 * @param nSteps The number of steps in the ramp iteration.
 * @param del The delay in milliseconds between each step of the ramp.
 * @return void
 */
uint8_t RAMPS::bufferRampIteration(uint8_t channelsDac[4], double vi[4], double nSteps, double del) {

  double dv_j;

  //Initial delay before first step
  delay(del);

  //Initial reading before first step
  adc.bufferRampFullReading();


  for (int i = 0; i < nSteps; i++) {
    for (int j = 0; j < 4; j++) {
      if (channelsDac[j] == 1) {
        dv_j = dv[j];

        //Update channel i voltage
        dac.setVoltage(j, vi[j] + ((i+1)*dv_j), false);
        //Serial.print("vReadings[j]: ");
        //Serial.println(dac.vReadings[j]);
      }
    }
    //set LDAC pin to low to update all channels simultaneously
    dac.updateAnalogOutputs();
    
    //delay of input delay
    delay(del);

    //Read
    adc.bufferRampFullReading();
  }
  return 0;
}

/**
 * @brief Performs a simple ramp for the specified channels on the RAMPS board.
 *
 * This function performs a simple ramp for the specified channels on the RAMPS board. The ramp is controlled by the
 * 'nSteps' parameter, which determines the number of steps in the ramp. The 'vi' array contains the initial voltage
 * values for each corresponding channel, and the 'vf' array contains the final voltage values. The 'del' parameter
 * specifies the delay in milliseconds between each step of the ramp. The ramp iteration updates the voltage of each
 * channel incrementally based on the voltage step size previously calculated and stored in the 'dv' array. The updated
 * voltage values are applied to the DAC channels using the 'setVi' function. If the 'buffer' parameter is set to true,
 * the ramp iteration will use the 'bufferRampIteration' function, which includes ADC readings after each step. If set
 * to false, the 'simpleRampIteration' function will be used instead. Optional Serial print statements can be uncommented
 * for debugging or logging purposes.
 *
 * @param channelsDac An array indicating which channels to perform the ramp on.
 * @param vi An array of initial voltage values for each corresponding channel.
 * @param vf An array of final voltage values for each corresponding channel.
 * @param nSteps The number of steps in the ramp.
 * @param del The delay in milliseconds between each step of the ramp.
 * @param buffer Indicates whether to use buffer ramp iteration (true) or simple ramp iteration (false).
 * @return void
 */
uint8_t RAMPS::simpleRamp(uint8_t channelsDac[4], double vi[4], double vf[4], double nSteps, double del, bool buffer) {
  
  //Serial.println("| simpleRamp : ");

  //double prevVoltage;

  calcDv(channelsDac, vi, vf, nSteps);

  // Serial.print("dv : ");
  //   for (int i = 0; i < 4; i++) {
  //      Serial.print(dv[i], 6);
  //      Serial.print(", ");
  //   } 

  setVi(channelsDac, vi);
  // Serial.println("");

  // Serial.print("Initial voltages: ");
  //   for (int i = 0; i < 4; i++) {
  //      Serial.print(dac.readVoltage(i));
  //      Serial.print(", ");
  //   } 

  if (buffer) {bufferRampIteration(channelsDac, vi, nSteps, del);}
  else {simpleRampIteration(channelsDac, vi, nSteps, del);}

  
  //Serial.println("");

}
