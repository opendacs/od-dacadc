#include "../include/ad4115.h"
#include <stdint.h>
#include <SPI.h>
#include <cstdlib>
#include <Arduino.h>
using namespace std;

AD4115::AD4115(uint8_t sync_pin, uint8_t drdy_pin) {

}


uint8_t AD4115::reset_adc(void) {

	adc_sync = 32;
	drdy = 28;

	pinMode(32, OUTPUT);
	pinMode(28, INPUT);
	pinMode(50, OUTPUT);
	digitalWrite(50, HIGH);
	digitalWrite(adc_sync, HIGH);
	SPI.beginTransaction(adcSettings);
  	
  	for (int i = 0; i < 8; i++) {
    	digitalWrite(adc_sync, LOW);
    	SPI.transfer(0xFF);
    	digitalWrite(adc_sync, HIGH);
    }
  	delay(1);
  	SPI.endTransaction();

    return 0;
}

void AD4115::wait_drdy(void) {
	while (digitalRead(drdy) == HIGH) {} 
}

spi_utils::Message AD4115::disable_all_channels_Msg(void) {

	spi_utils::Message data;

	for (int chl = 0; chl < 16; chl++) {
		uint8_t count = chl;
		spi_utils::Message msg = config_channel_Msg(chl, 0, 0, 0, 1);
    	
    	// Serial.print("count ");
    	// Serial.println(count);
    	// Serial.println(msg.msg[0]);
    	// Serial.println(msg.msg[1]);
    	// Serial.println(msg.msg[2]);

	    // 4 LSB are Channel address
	    data.data[3 * chl] = msg.msg[0]; 
	    // Serial.println(3 * chl);
	    // Serial.println(msg.msg[3 * chl]);
	    
	    // Disable channel 
	    data.data[(3 * chl) + 1] = msg.msg[1];
	    // Serial.println((3 * chl) + 1);
	    // Serial.println(msg.msg[(3 * chl) + 1]);
	    
	    //Irrelevant
	    data.data[(3 * chl) + 2] = msg.msg[2];
	    // Serial.println((3 * chl) + 2);
	    // Serial.println(msg.msg[(3 * chl) + 1]);
    }

    for (int i = 0; i < 16; i++) {
    	Serial.print("Channel ");
    	Serial.println(i);
    	Serial.println(channel_states[i]);
    }
    return data;
}

uint8_t AD4115::disable_all_channels(void) {

	spi_utils::Message data = disable_all_channels_Msg();
	
	SPI.beginTransaction(adcSettings);

	data.block_size = 48;
    data.n_blocks = 1;

	for (uint8_t block = 0; block < data.n_blocks; block++) {

        digitalWrite(adc_sync, LOW);

        for (uint8_t db = 0; db < data.block_size; db++) {

            SPI.transfer(data.data[block * data.block_size + db]);
            Serial.println(data.data[block * data.block_size + db]);
        }
        digitalWrite(adc_sync, HIGH);
    }
    SPI.endTransaction();

    return 0;
}

spi_utils::Message AD4115::config_channel_Msg(uint8_t channel, uint8_t state, uint8_t setup, uint8_t input_1, uint8_t input_2) {

	spi_utils::Message msg;
	
	uint16_t channel_data;
	uint8_t channel_reg;
	uint8_t channel_setup;
	uint8_t channel_inputs;
	
	//Channel register address
	channel_reg = 0x10 + channel;

	//Enable/disable channel
	if (state == 1 || state == 0) {
		if (state == 1) {
			channel_data = (channel_data | 1);
			channel_states[channel] = 1;
		}
		else {
			channel_states[channel] = 0;
		}
		
		if (0 <= setup <= 7) {
			channel_data = ((channel_data << 3) + setup);

			//Reserved bits	
			channel_data = (channel_data << 2);

			if (((abs(input_1 - input_2) == 1) && (((input_1 + input_2) - 1) % 4) == 0) || (input_2 == 16)) {
				
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

	uint16_t channel_setup_mask = 0xFF00;
	uint16_t channel_inputs_mask = 0x00FF;

	channel_setup = ((channel_data & channel_setup_mask) >> 8);
	channel_inputs = ((channel_data & channel_inputs_mask) >> 0);
	
	msg.msg[0] = channel_reg;
	Serial.println("channel_reg");
	Serial.println(channel_reg);
	
	msg.msg[1] = channel_setup;
	Serial.println("channel_setup");
	Serial.println(channel_setup);
	
	msg.msg[2] = channel_inputs;
	Serial.println("channel_inputs");
	Serial.println(channel_inputs);

	return msg;
}	

// In general_config
uint8_t AD4115::config_channel(uint8_t channel, uint8_t state,  uint8_t setup, uint8_t input_1, uint8_t input_2) {

	spi_utils::Message msg = config_channel_Msg(channel, state, setup, input_1, input_2);

	SPI.beginTransaction(adcSettings);

	msg.block_size = 3;
    msg.n_blocks = 1;

	for (uint8_t block = 0; block < msg.n_blocks; block++) {

        digitalWrite(adc_sync, LOW);

        for (uint8_t db = 0; db < msg.block_size; db++) {

            SPI.transfer(msg.msg[block * msg.block_size + db]);
            Serial.println("config_channel");
            Serial.println(db);
            Serial.println(msg.msg[block * msg.block_size + db]);
        }
        //digitalWrite(adc_sync, HIGH);
    }
    SPI.endTransaction();

    for (int i = 0; i < 16; i++) {
    	Serial.print("Channel ");
    	Serial.println(i);
    	Serial.println(channel_states[i]);
    }

    return 0;
}

uint32_t AD4115::twoByteToInt(byte db1, byte db2) {

	return (uint32_t) ((db1 << 8) | db2);
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
    msg.msg[0] = 0x20; // Send 0010 0000 == 32

    // Enable/disable input buffers [8:9] -- 11 (e.g. enabled)
    // Enable/disable REF(-) input buffer [10] -- 0 (e.g. disabled)
    // Enable/disable REF(+) input buffer [11] -- 0 (e.g. disabled)
    // Bipolar/unipolar output coding [12] -- 1 (e.g. bipolar)
    // Reserved [13:15] -- 000
    msg.msg[1] = 0x13; // Send 0001 0011 = 19
    
    // Reserved [0:3] -- 0000
    // Select ref source [4:5] -- 00 (e.g. external ref)
    // Reserved [6:7] -- 00
    msg.msg[2] = 0x00; // Send 0000 0000 = 0

    return msg;
}

//In general_config
uint8_t AD4115::setup_config(void) {
	
	spi_utils::Message msg = setup_config_Msg();
	
	msg.block_size = 3;
    msg.n_blocks = 1;
	
	SPI.beginTransaction(adcSettings);
	
	for (uint8_t block = 0; block < msg.n_blocks; block++) {

        for (uint8_t db = 0; db < msg.block_size; db++) {

            SPI.transfer(msg.msg[block * msg.block_size + db]);
        }
    }
    SPI.endTransaction();

    return 0;
}

spi_utils::Message AD4115::interface_mode_Msg(void) {

	spi_utils::Message msg;

    // 000010 -- Address [0:5]
    // 0 -- WRITE [6]
    // 0 -- WEN [7]
    msg.msg[0] = 0x02; // Send 0000 0010

    // DOUT_RESET [8] -- 0 (e.g. disabled)
    // Reserved [9:10] -- 00
    // Drive strength of DOUT/DRY pin [11] -- 0 (e.g. disabled)
    // ALT_SYNC [12] -- 0 (e.g. disabled)
    // Reserved [13:15] -- 000
    msg.msg[1] = 0x00; // Send 0000 0000

    // Change ADC to 16 bits [0] -- 0 (e.g. 24 bits)
    // Reserved [1] -- 0
    // CRC protection [2:3] -- 00 (e.g. disabled)
    // Reserved [4] -- 0
    // Register intgrity checker [5] -- 0 (e.g. disabled)
    // DATA_STAT [6] -- 0 (e.g. disabled)
    // Enables continue read mode [7] -- 0 (e.g. disabled)
    msg.msg[2] = 0x00; // Send 0000 0000

    return msg;
}

//In general config
uint8_t AD4115::interface_mode(void) {

	spi_utils::Message msg = interface_mode_Msg();

	msg.block_size = 3;
	msg.n_blocks = 1;

	SPI.beginTransaction(adcSettings);

	for (uint8_t block = 0; block < msg.n_blocks; block++) {

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
    msg.msg[0] = 0x01; // Send 0000 0001 

    // Delay [8:10] -- 000 (e.g. 0 microsecs)
    // Reserved [11:12] -- 00
    // ON if single channel active [13] -- 0 (e.g. disabled)
    // Reserved [14] -- 0
    // REF_EN [15] -- 0 (e.g. disabled)
    msg.msg[1] = 0x00; // Send 0000 0000

    // Reserved [0:1] -- 00
    // ADC clock source [2:3] -- 11 kk
    // Operating mode [4:6] -- 001 (e.g. single conversion mode)
    // Reserved [7] -- 0
    msg.msg[2] = 0x1C; // Send 0001 1100

    return msg;
}

//In full_reading
void AD4115::adc_mode(void) {

	spi_utils::Message msg = adc_mode_Msg();

	msg.block_size = 3;
	msg.n_blocks = 1;

	SPI.beginTransaction(adcSettings);

	for (uint8_t block = 0; block < msg.n_blocks; block++) {

        digitalWrite(adc_sync, LOW);

        for (uint8_t db = 0; db < msg.block_size; db++) {

            SPI.transfer(msg.msg[block * msg.block_size + db]);
        }
    }
    SPI.endTransaction();
}
//Optional--not in use
uint8_t AD4115::update_channel_states(void) {

	uint8_t state_mask = 0x80; //1000 0000

	spi_utils::Message msg;

	msg.block_size = 16;
	msg.n_blocks = 1;

	SPI.beginTransaction(adcSettings);

	for (uint8_t block = 0; block < msg.n_blocks; block++) {

        digitalWrite(adc_sync, LOW);

        for (uint8_t db = 0; db < msg.block_size; db++) {

            SPI.transfer(0x50 + db);
    		uint8_t db1 = SPI.transfer(0x00);
    		uint8_t db2 = SPI.transfer(0x00); //irrelevant

    		uint8_t state = (state_mask & db1);

    		if (state == 0x80) {
    			channel_states[db] = 1;
    		}
    		else {
    			channel_states[db] = 0;
    		}
        }
        digitalWrite(adc_sync, HIGH);
    }
    SPI.endTransaction();

    return 0;	
}

double AD4115::voltageMap(double decimal) {
	return ((double) ((decimal / 8388608-1) * 25));
}

// Returns a 24 bit integer (between 0 - 2^24)
double AD4115::threeByteToInt(uint8_t db1, uint8_t db2, uint8_t db3) {
	return ( (double) ( ( ( db1 << 8 ) + db2 ) << 8 ) + db3 );
}

spi_utils::Message AD4115::data_reading_Msg(void) {

	spi_utils::Message msg;

	msg.msg[0] = 0x44;
	msg.msg[1] = 0x00;
	msg.msg[2] = 0x00;
	msg.msg[3] = 0x00;

	return msg;
}

//In full_reading
void AD4115::data_reading(void) {

	spi_utils::Message msg = data_reading_Msg();

	msg.block_size = 4;
	msg.n_blocks = 1;

	SPI.beginTransaction(adcSettings);

	for (uint8_t block = 0; block < msg.n_blocks; block++) {

        for (uint8_t db = 0; db < msg.block_size; db++) {

        	if (db == 0) {
        		SPI.transfer(msg.msg[block * msg.block_size + db]);
        	}
            else {
            	data_read[db - 1] = SPI.transfer(msg.msg[block * msg.block_size + db]);
            } 
        }
    }
    SPI.endTransaction();
}

double AD4115::full_reading(void) {

	adc_mode();

	for (int i = 0; i < 16; i++) {
		if (channel_states[i] == 1) {
			wait_drdy();
			data_reading();

			channel_decimals[i] = threeByteToInt(data_read[0], data_read[1], data_read[2]);
    		channel_voltages[i] = voltageMap(channel_decimals[i]);

    		// Serial.print("Channel ");
    		// Serial.print(i);
    		// Serial.print(":");
    		// Serial.print(channel_voltages[i]);
    		// Serial.println("V");
		}
	}

	digitalWrite(adc_sync, HIGH);

	for (int i = 0; i < 16; i++) {
		Serial.println(i);
		Serial.println(channel_voltages[i],6);

	}
	return 0;
}	