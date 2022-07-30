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

private:
	uint32_t twoByteToInt(byte db1, byte db2);
	uint32_t threeByteToInt(byte db1, byte db2, byte db3);
	uint8_t single_reading(uint8_t channel);

public:
	AD4115(void) = default;
	SPISettings adcSettings = SPISettings(1000000, MSBFIRST, SPI_MODE3);
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
	uint8_t adc_mode(void);

	//Constructor
	AD4115(uint8_t sync_pin);

	//Numbering system conversions
}

#endif // AD4115_H
