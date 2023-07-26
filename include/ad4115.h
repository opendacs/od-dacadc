#ifndef AD4115_H
#define AD4115_H
#include <SPI.h>
#include <stdint.h>
#include "utils.h"

using namespace std;

class AD4115 
{
protected:
	virtual spi_utils::Message disableAllChannelsMsg(void);
	virtual spi_utils::Message setupConfigMsg(void);
	virtual spi_utils::Message configChannelMsg(uint8_t channel, uint8_t state, uint8_t setup, uint8_t input_1, uint8_t input_2);
	virtual spi_utils::Message interfaceModeMsg(void);
	virtual spi_utils::Message adcModeMsg(void);
	virtual spi_utils::Message dataReadingMsg(void);

private:
	//Functions
	uint32_t twoByteToInt(byte db1, byte db2);
	double threeByteToInt(uint8_t db1, uint8_t db2, uint8_t db3);
	void intToThreeByte(uint32_t decimal);
	double voltageMap(double decimal);
	void waitDrdy(void);
	
	//Variables
	int _channelStates[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	double _channelDecimals[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	double _channelVoltages[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	uint8_t _dataRead[3];
	uint8_t _adcSync;
	uint8_t _drdy;

public:
	//Constructor
	AD4115(uint8_t adcSync, uint8_t drdy);
	AD4115(void) = default;
	SPISettings adcSettings = SPISettings(10000000, MSBFIRST, SPI_MODE3);
	///
	///
	///
	/// Config for a specific channel
	/// Selects the channel number to modify- ranges from 0 to 15
	/// State: 0 disables channel - 1 enables channel
	///	Setup: which to use. Ranges from 0 to 7
	/// input_1 and input_2: pair of input voltages in ADC
	/// --> input_1: int value from 0 to 15
	/// --> input_2: int value from 0 to 16 -- 16 is VINCOM
	///
	uint8_t configChannel(uint8_t channel, uint8_t state, uint8_t setup, uint8_t input_1, uint8_t input_2);
	uint8_t disableAllChannels(void);
	uint8_t setupConfig(void);
	uint8_t generalConfig(uint8_t channel, uint8_t state, uint8_t setup, uint8_t input_1, uint8_t input_2);
	uint8_t readId(void);
	uint8_t interfaceMode(void);
	void adcMode(void);
	void dataReading(void);
	//Returns array of size 16--0 if deactivated, 1 if activated
	uint8_t updateChannelStates(void);
	//Iterates through array and counts the 1s--returns integer
	double fullReading(void);
	double bufferRampFullReading(void);
	uint8_t resetAdc(void);

	//Test functions
	uint8_t configChannelsTest(void);

};

#endif // AD4115_H
