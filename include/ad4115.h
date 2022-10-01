#ifndef AD4115_H
#define AD4115_H
#include <SPI.h>
#include <stdint.h>
#include "utils.h"
using namespace std;

class AD4115 
{
protected:
	virtual spi_utils::Message disable_all_channels_Msg(void);
	virtual spi_utils::Message setup_config_Msg(void);
	virtual spi_utils::Message config_channel_Msg(uint8_t channel, uint8_t state, uint8_t setup, uint8_t input_1, uint8_t input_2);
	virtual spi_utils::Message interface_mode_Msg(void);
	virtual spi_utils::Message adc_mode_Msg(void);
	virtual spi_utils::Message data_reading_Msg(void);

private:
	uint32_t twoByteToInt(byte db1, byte db2);
	double threeByteToInt(uint8_t db1, uint8_t db2, uint8_t db3);
	double voltageMap(double decimal);
	int channel_states[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	double channel_decimals[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	double channel_voltages[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	uint8_t data_read[3];
	void wait_drdy(void);
	uint8_t adc_sync;
	uint8_t drdy;

public:
	//Constructor
	AD4115(uint8_t adc_sync, uint8_t drdy);
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
	uint8_t config_channel(uint8_t channel, uint8_t state, uint8_t setup, uint8_t input_1, uint8_t input_2);
	uint8_t disable_all_channels(void);
	uint8_t setup_config(void);
	uint8_t general_config(uint8_t channel, uint8_t state, uint8_t setup, uint8_t input_1, uint8_t input_2);
	uint8_t read_id(void);
	uint8_t interface_mode(void);
	void adc_mode(void);
	void data_reading(void);
	//Returns array of size 16--0 if deactivated, 1 if activated
	uint8_t update_channel_states(void);
	//Iterates through array and counts the 1s--returns integer
	double full_reading(void);
	uint8_t reset_adc(void);

};

#endif // AD4115_H
