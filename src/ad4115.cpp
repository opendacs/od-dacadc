#include "../include/ad4115.h"
#include <stdint.h>
#include <SPI.h>
#include <cstdlib>
using namespace std;

AD4115::AD4115(uint8_t sync_pin) {

	uint8_t adc_sync = sync_pin;
}

spi_utils::Message AD4115::config_channel_Msg(uint8_t channel, uint8_t state, uint8_t setup, uint8_t input_1, uint8_t input_2) {

	spi_utils::Message msg;

	byte channel_reg;
	long channel_data;
	byte channel_setup;
	byte channel_inputs;
	
	//Channel register address
	channel_reg = 0x10 + channel;

	//Enable/disable channel
	if (state == 1 || state == 0) {
		channel_data = (channel_data | 1);

		if (0 <= setup <= 7) {
			channel_data = ((channel_data << 3) + setup);

			//Reserved bits	
			channel_data = (channel_data << 2);

			if (abs(input_1 - input_2) == 1) {
				
				if (0 <= input_1 <= 15) {
					
					if (0 <= input_2 <= 16) {
						channel_data = ((channel_data << 5) + input_1);

						if (input_2 == 16) {
							channel_data = ((channel_data << 1) + 1);
							channel_data = (channel_data << 4);
						}
						else {
							channel_data = ((channel_data << 5) + input_2);
						}
					}
					else {
						Serial.println("INPUT 2 OUT OF RANGE");
					}
				}
				else {
					Serial.println("INPUT 1 OUT OF RANGE");
				}
			}
			else {
				Serial.println("INVALID INPUTS PAIR");
			}
		}
		else {
			Serial.println("INVALID SETUP");
		}
	}
	else {
		Serial.println("INVALID STATE");
	}

	uint8_t channel_setup_mask = 0xF0;
	uint8_t channel_inputs_mask = 0x0F;

	channel_setup = ((channel_data & channel_setup_mask) >> 4);
	channel_inputs = ((channel_data & channel_inputs_mask) >> 0);
	
	msg.msg[0] = channel_reg;
	msg.msg[1] = channel_setup;
	msg.msg[2] = channel_inputs;

	return msg;
}	

uint8_t AD4115::config_channel(uint8_t channel, uint8_t state,  uint8_t setup, uint8_t input_1, uint8_t input_2) {

	spi_utils::Message msg = config_channel_Msg(channel, state, setup, input_1, input_2);

	SPI.beginTransaction(adcSettings);

	msg.block_size = 3;
    msg.n_blocks = 1;

	for (uint8_t block = 0; block < msg.n_blocks; block++) {

        digitalWrite(adc_sync, LOW);

        for (uint8_t db = 0; db < msg.block_size; db++) {

            SPI.transfer(msg.msg[block * msg.block_size + db]);
        }
        digitalWrite(adc_sync, HIGH);
    }
    SPI.endTransaction();

    return 0;
}


spi_utils::Message AD4115::disable_all_channels_Msg(void) {

	spi_utils::Message msg;

	uint8_t data[3];

	for (int chl = 0; chl < 16; i++) {

		data = config_channel_Msg(chl, 0, 0, 0, 1)
    
	    // 4 LSB are Channel address
	    msg.msg[3 * chl] = data[0]; 
	    
	    // Disable channel 
	    msg.msg[(3 * chl) + 1] = data[1];
	    
	    //Irrelevant
	    msg.msg[(3 * chl) + 2] = data[2];
    }

    return msg;
}

uint8_t AD4115::disable_all_channels(void) {

	spi_utils::Message msg = disable_all_channels_Msg();
	
	SPI.beginTransaction(adcSettings);

	msg.block_size = 48;
    msg.n_blocks = 1;

	for (uint8_t block = 0; block < msg.n_blocks; block++) {

        digitalWrite(adc_sync, LOW);

        for (uint8_t db = 0; db < msg.block_size; db++) {

            SPI.transfer(msg.msg[block * msg.block_size + db]);
        }
        digitalWrite(adc_sync, HIGH);
    }
    SPI.endTransaction();

    return 0;
}

uint32_t AD4115::twoByteToInt(byte db1, byte db2) {

	return = (uint32_t) ((db1 << 8) | db2)
}

uint8_t AD4115::read_id(void) {

	SPI.beginTransaction(adcSettings);
	
	digitalWrite(adc_sync, LOW);
	
	SPI.transfer(0x47); //READ to ID register address
    uint8_t ID1 = SPI.transfer(0x00);
    uint8_t ID2 = SPI.transfer(0x00);
    
    digitalWrite(adc_sync, HIGH);

    uint8_t ID = twoByteToInt(ID1, ID2);
    
    Serial.println(ID);
}

spi_utils::Message AD4115::setup_config_Msg(void) {

	spi_utils::Message msg;
    
    // 100000 -- Address [0:5] (e.g. Setup 0)
    // 0 -- WRITE [6]
    // 0 -- WEN [7]
    msg[0] = 0x20; // Send 0010 0110

    // Enable/disable input buffers [8:9] -- 11 (e.g. enabled)
    // Enable/disable REF(-) input buffer [10] -- 0 (e.g. disabled)
    // Enable/disable REF(+) input buffer [11] -- 0 (e.g. disabled)
    // Bipolar/unipolar output coding [12] -- 1 (e.g. bipolar)
    // Reserved [13:15] -- 000
    msg[1] = 0x13; // Send 0001 0011
    
    // Reserved [0:3] -- 0000
    // Select ref source [4:5] -- 00 (e.g. external ref)
    // Reserved [6:7] -- 00
    msg[2] = 0x00; // Send 0000 0000

    return msg;
}

uint8_t AD4115::setup_config(void) {
	
	spi_utils::Message msg = setup_config_Msg();
	
	msg.block_size = 3;
    msg.n_blocks = 1;
	
	SPI.beginTransaction(adcSettings);
	
	for (uint8_t block = 0; block < msg.n_blocks; block++) {

        digitalWrite(adc_sync, LOW);

        for (uint8_t db = 0; db < msg.block_size; db++) {

            SPI.transfer(msg.msg[block * msg.block_size + db]);
        }
        digitalWrite(adc_sync, HIGH);
    }
    SPI.endTransaction();

    return 0;
}

spi_utils::Message AD4115::interface_mode_Msg(void) {

	spi_utils::Message msg;

    // 000010 -- Address [0:5]
    // 0 -- WRITE [6]
    // 0 -- WEN [7]
    msg[0] = 0x02; // Send 0000 0010

    // DOUT_RESET [8] -- 0 (e.g. disabled)
    // Reserved [9:10] -- 00
    // Drive strength of DOUT/DRY pin [11] -- 0 (e.g. disabled)
    // ALT_SYNC [12] -- 0 (e.g. disabled)
    // Reserved [13:15] -- 000
    msg[1] = 0x00; // Send 0000 0000

    // Change ADC to 16 bits [0] -- 0 (e.g. 24 bits)
    // Reserved [1] -- 0
    // CRC protection [2:3] -- 00 (e.g. disabled)
    // Reserved [4] -- 0
    // Register intgrity checker [5] -- 0 (e.g. disabled)
    // DATA_STAT [6] -- 0 (e.g. disabled)
    // Enables continue read mode [7] -- 0 (e.g. disabled)
    msg[2] = 0x00; // Send 0000 0000

    return msg;
}

uint8_t AD4115::interface_mode(void) {

	spi_utils::Message msg = interface_mode_Msg();

	msg.block_size = 3;
	msg.n_blocks = 1;

	SPI.beginTransaction(adcSettings);

	for (uint8_t block = 0; block < msg.n_blocks; block++) {

        digitalWrite(adc_sync, LOW);

        for (uint8_t db = 0; db < msg.block_size; db++) {

            SPI.transfer(msg.msg[block * msg.block_size + db]);
        }
        digitalWrite(adc_sync, HIGH);
    }
    SPI.endTransaction();

    return 0;
}

uint8_t AD4115::general_config(uint8_t channel, uint8_t state, uint8_t setup, uint8_t input_1, uint8_t input_2) {

	uint8_t db1 = config_channel(channel, state, setup, input_1, input_2);
	uint8_t db2 = setup_config();
	uint8_t db3 = interface_mode();

	return 0;
}
spi_utils::Message AD4115::adc_mode_Msg() {
	
	spi_utils::Message msg;

    // 000001 -- Address [0:5]
    // 0 -- WRITE [6]
    // 0 -- WEN [7]
    msg[0] = 0x01; // Send 0000 0001 

    // Delay [8:10] -- 000 (e.g. 0 microsecs)
    // Reserved [11:12] -- 00
    // ON if single channel active [13] -- 0 (e.g. disabled)
    // Reserved [14] -- 0
    // REF_EN [15] -- 0 (e.g. disabled)
    msg[1] = 0x00; // Send 0000 0000

    // Reserved [0:1] -- 00
    // ADC clock source [2:3] -- 11 kk
    // Operating mode [4:6] -- 001 (e.g. single conversion mode)
    // Reserved [7] -- 0
    msg[2] = 0x1C; // Send 0001 1100
}

spi_utils::Message AD4115::data_reading_Msg() {

	spi_utils::Message msg;

	msg[0] = 0x44;
	msg[1] = 0x00;
	msg[2] = 0x00;
	msg[3] = 0x00;

	return msg;
}

uint8_t AD4115::data_reading(void) {

	spi_utils::Message msg = data_reading_Msg();

	msg.block_size = 4;
	msg.n_blocks = 1;

	uint8_t data[3];

	SPI.beginTransaction(adcSettings);

	for (uint8_t block = 0; block < msg.n_blocks; block++) {

        digitalWrite(adc_sync, LOW);

        for (uint8_t db = 0; db < msg.block_size; db++) {

        	if (db > 0) {
        		data[db - 1] = SPI.transfer(msg.msg[block * msg.block_size + db]);
        	}
            else {
            	SPI.transfer(msg.msg[block * msg.block_size + db]);
            } 
        }
        digitalWrite(adc_sync, HIGH);
    }
    SPI.endTransaction();

	return data;
}

uint8_t AD4115::adc_mode() {

	spi_utils::Message msg = adc_mode_Msg();

	msg.block_size = 3;
	msg.n_blocks = 1;

	SPI.beginTransaction(adcSettings);

	for (uint8_t block = 0; block < msg.n_blocks; block++) {

        digitalWrite(adc_sync, LOW);

        for (uint8_t db = 0; db < msg.block_size; db++) {

            SPI.transfer(msg.msg[block * msg.block_size + db]);
        }
        digitalWrite(adc_sync, HIGH);
    }
    SPI.endTransaction();

    return 0;
}

double AD4115::voltageMap(uint8_t decimal) {
	return ((decimal / 8388608-1) * 25);
}

// Returns a 24 bit integer (between 0 - 2^24)
uint32_t AD4115::threeByteToInt(byte db1, byte db2, byte db3) {
	return ( (int) ( ( ( DB1 << 8 ) + DB2 ) << 8 ) + DB3 );
}

double AD4115::single_reading(uint8_t channel) {
	uint8_t db1 = adc_mode();
	uint8_t data[3] = data_reading();

	uint8_t decimal = threeByteToInt(data[0], data[1], data[2]);
    double voltage = voltageMap(decimal);
	
	return voltage;
}	