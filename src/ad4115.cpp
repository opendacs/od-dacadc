#include "../include/ad4115.h"
#include <stdint.h>
#include <SPI.h>
#include <cstdlib>
#include <Arduino.h>
using namespace std;

/**
 * @brief Constructor for the AD4115 class.
 *
 * Initializes the AD4115 object with the provided sync and DRDY pins.
 * Sets the necessary pin modes and initial states for proper communication with the AD4115 ADC.
 *
 * @param sync_pin The pin connected to the ADC SYNC pin.
 * @param drdy_pin The pin connected to the ADC DRDY (data ready) pin.
 */
AD4115::AD4115(uint8_t sync_pin, uint8_t drdy_pin) {
	_adcSync = sync_pin;
	_drdy = drdy_pin;

	pinMode(32, OUTPUT);
	pinMode(28, INPUT);
	pinMode(50, OUTPUT);
	digitalWrite(50, HIGH);
	digitalWrite(_adcSync, HIGH);
}

/**
 * @brief Resets the AD4115 ADC.
 *
 * This function performs a reset operation on the AD4115 ADC. It begins an SPI transaction,
 * sends a sequence of reset commands, and ends the transaction. The function includes a delay
 * after the reset commands to allow time for the reset operation to complete.
 *
 * @return 0 indicating successful execution.
 */
uint8_t AD4115::resetAdc(void) {
	
	SPI.beginTransaction(adcSettings);
  	
  	for (int i = 0; i < 8; i++) {
    	digitalWrite(_adcSync, LOW);
    	SPI.transfer(0xFF);
    	digitalWrite(_adcSync, HIGH);
    }
  	delay(1);
  	SPI.endTransaction();

    return 0;
}

/**
 * @brief Waits for the DRDY (data ready) signal of the AD4115 ADC.
 *
 * This function waits until the DRDY pin of the AD4115 ADC transitions to a LOW state,
 * indicating that new data is available. It uses a busy-wait loop to continuously check
 * the state of the DRDY pin. The function exits when the DRDY pin transitions to a LOW state.
 */
void AD4115::waitDrdy(void) {
	while (digitalRead(_drdy) == HIGH) {} 
}

/**
 * @brief Creates a message to disable all channels of the AD4115. Calls configChannelMsg()
 *
 * This function generates a message to disable all 16 channels of the AD4115 ADC.
 * It iterates through each channel, configures the appropriate values for disabling,
 * and constructs the resulting message.
 *
 * @return A spi_utils::Message object containing the message to disable all channels.
 */
spi_utils::Message AD4115::disableAllChannelsMsg(void) {

	spi_utils::Message data;

	for (int chl = 0; chl < 16; chl++) {
		uint8_t count = chl;
		spi_utils::Message msg = configChannelMsg(chl, 0, 0, 0, 1);

	    // 4 LSB are Channel address
	    data.data[3 * chl] = msg.msg[0]; 

	    // Disable channel 
	    data.data[(3 * chl) + 1] = msg.msg[1];

	    //Irrelevant
	    data.data[(3 * chl) + 2] = msg.msg[2];
    }
    return data;
}

/**
 * @brief Disables all channels of the AD4115 ADC.
 *
 * This function disables all 16 channels of the AD4115 ADC by sending a message
 * generated by the disableAllChannelsMsg() function. It begins the SPI transaction,
 * transfers the data in blocks, and ends the transaction.
 *
 * @return 0 indicating successful execution.
 */
uint8_t AD4115::disableAllChannels(void) {

	spi_utils::Message data = disableAllChannelsMsg();
	
	SPI.beginTransaction(adcSettings);

	data.blockSize = 48;
    data.nBlocks = 1;

	for (uint8_t block = 0; block < data.nBlocks; block++) {

        digitalWrite(_adcSync, LOW);

        for (uint8_t db = 0; db < data.blockSize; db++) {

            SPI.transfer(data.data[block * data.blockSize + db]);
        }
        digitalWrite(_adcSync, HIGH);
    }
    SPI.endTransaction();

    return 0;
}

/**
 * @brief Generates a configuration message for a specific channel of the AD4115 ADC.
 *
 * This function generates a configuration message for the specified channel of the AD4115 ADC.
 * The message includes the channel register address, channel setup, and channel input configuration.
 * The function performs various checks and validations to ensure the inputs are within valid ranges.
 * Only valid combinations (order doesn't matter) for [input_1, input_2]: [0, 1], [2, 3], [4, 5], [6, 7], [8, 9], [10, 11], [12, 13], [14, 15] 
 * When using VINCOM order does matter: [i, 16], for 0 <= i <= 15
 * 
 * @param channel The channel number to configure.
 * @param state   The state (1 = enable/ 0 = disable) of the channel.
 * @param setup   The setup value for the channel. For now, only setup 0 works.
 * @param input_1 The first input BNC for the channel.
 * @param input_2 The second input BNC for the channel. VINCOM: input_2 = 16
 * @return A spi_utils::Message object containing the generated configuration message.
 */
spi_utils::Message AD4115::configChannelMsg(uint8_t channel, uint8_t state, uint8_t setup, uint8_t input_1, uint8_t input_2) {

	spi_utils::Message msg;
	
	uint16_t channel_data = 0x0;
	uint8_t channel_reg = 0x0;
	uint8_t channel_setup = 0x0;
	uint8_t channel_inputs = 0x0;
	
	//Channel register address
	channel_reg = 0x10 + channel;

	//Enable/disable channel
	if (state == 1 || state == 0) {
		if (state == 1) {
			channel_data = (channel_data | 1);
			_channelStates[channel] = 1;
		}
		else {
			_channelStates[channel] = 0;
		}
		
		if (0 <= setup <= 7) {
			channel_data = ((channel_data << 3) + setup);

			//Reserved bits	
			channel_data = (channel_data << 2);

			if ((input_2 == 16) || ((abs(input_1 - input_2) == 1) && (((input_1 + input_2) - 1) % 4) == 0)) {
				
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
						msg.errorMessage();
						return msg;
					}
				}
				else {
					Serial.println("INPUT 1 OUT OF RANGE");
					msg.errorMessage();
					return msg;
				}
			}
			else {
				Serial.println("INVALID INPUTS PAIR");
				msg.errorMessage();
				Serial.println(msg.msg[0]);
				return msg;
			}
		}
		else {
			Serial.println("INVALID SETUP");
			msg.errorMessage();
			return msg;
		}
	}
	else {
		Serial.println("INVALID STATE");
		msg.errorMessage();
		return msg;
	}

	uint16_t channel_setup_mask = 0xFF00;
	uint16_t channel_inputs_mask = 0x00FF;

	channel_setup = ((channel_data & channel_setup_mask) >> 8);
	channel_inputs = ((channel_data & channel_inputs_mask) >> 0);
	
	msg.msg[0] = channel_reg;
	
	msg.msg[1] = channel_setup;
	
	msg.msg[2] = channel_inputs;

	return msg;
}	

/**
 * @brief Configures a channel of the AD4115 ADC.
 *
 * This function configures the specified channel of the AD4115 ADC using the provided parameters.
 * It generates a configuration message by calling the configChannelMsg() function and transfers
 * the message via SPI communication. The function also includes debug print statements for monitoring
 * the transfer process. After the configuration, it displays the channel states for all channels.
 *
 * @param channel  The channel number to configure.
 * @param state    The state (enable = 1/disable = 0) of the channel.
 * @param setup    The setup value for the channel.
 * @param input_1  The first input configuration for the channel.
 * @param input_2  The second input configuration for the channel.
 *
 * @return 0 indicating successful execution.
 */
uint8_t AD4115::configChannel(uint8_t channel, uint8_t state,  uint8_t setup, uint8_t input_1, uint8_t input_2) {

	spi_utils::Message msg = configChannelMsg(channel, state, setup, input_1, input_2);
	if (msg.msg[0]==0xFF){
		return 1;
	}

	SPI.beginTransaction(adcSettings);

	msg.blockSize = 3;
    msg.nBlocks = 1;

	for (uint8_t block = 0; block < msg.nBlocks; block++) {
 
		// Sync set to LOW, but not returned to HIGH
        digitalWrite(_adcSync, LOW);

        for (uint8_t db = 0; db < msg.blockSize; db++) {

            SPI.transfer(msg.msg[block * msg.blockSize + db]);
        }

        //Temporary HIGH just for debugging purposes
        //digitalWrite(_adcSync, HIGH);

    }
    SPI.endTransaction();

    return 0;
}

/**
 * @brief Converts two bytes to a 32-bit unsigned integer.
 *
 * This function takes two bytes as input and combines them to form a 32-bit unsigned integer.
 * It performs a bitwise shift operation to concatenate the bytes and returns the resulting value.
 *
 * @param db1 The most significant byte.
 * @param db2 The least significant byte.
 *
 * @return A 32-bit unsigned integer formed by combining the input bytes.
 */
uint32_t AD4115::twoByteToInt(byte db1, byte db2) {

	return (uint32_t) ((db1 << 8) | db2);
}

/**
 * @brief Reads the ID register of the AD4115 ADC.
 *
 * This function reads the ID register of the AD4115 ADC via SPI communication.
 * It sends the appropriate commands to the ADC, receives the ID bytes, and combines them
 * to form a single 8-bit ID value. The ID value is then printed to the Serial monitor.
 *
 * @return None.
 */
uint8_t AD4115::readId(void) {

	SPI.beginTransaction(adcSettings);
	
	digitalWrite(_adcSync, LOW);
	
	SPI.transfer(0x47); //READ to ID register address
    uint8_t ID1 = SPI.transfer(0x00);
    uint8_t ID2 = SPI.transfer(0x00);
    
    digitalWrite(_adcSync, HIGH);

    uint8_t ID = twoByteToInt(ID1, ID2);
    
    Serial.println(ID);
}

/**
 * @brief Generates a setup configuration message for the AD4115 ADC.
 *
 * This function generates a setup configuration message for the AD4115 ADC. The message
 * includes specific values for different configuration parameters, such as address, buffer
 * enable/disable settings, output coding, and reference source selection. The function
 * constructs the message by assigning values to the corresponding elements of the spi_utils::Message
 * object and returns the resulting message.
 *
 * @return A spi_utils::Message object containing the generated setup configuration message.
 */
spi_utils::Message AD4115::setupConfigMsg(void) {

	spi_utils::Message msg;
    
	// 0 -- WEN [7]
	// 0 -- WRITE [6]
    // 100000 -- Address [0:5] (e.g. Setup 0)
    msg.msg[0] = 0x20; // Send 0010 0000 == 32

	// Reserved [13:15] -- 000
	// Bipolar/unipolar output coding [12] -- 1 (e.g. bipolar)
	// Enable/disable REF(+) input buffer [11] -- 1 (e.g. enabled)
	// Enable/disable REF(-) input buffer [10] -- 1 (e.g. enabled)
    // Enable/disable input buffers [8:9] -- 11 (e.g. enabled)
    msg.msg[1] = 0x1F; // Send 0001 1111 = 31
    
	// Reserved [6:7] -- 00
	// Select ref source [4:5] -- 00 (e.g. external ref)
    // Reserved [0:3] -- 0000
    msg.msg[2] = 0x00; // Send 0000 0000 = 0

    return msg;
}

/**
 * @brief Generates a setup configuration message for the AD4115 ADC.
 *
 * This function generates a setup configuration message for the AD4115 ADC. The message
 * includes specific values for different configuration parameters, such as address, buffer
 * enable/disable settings, output coding, and reference source selection. The function
 * constructs the message by assigning values to the corresponding elements of the spi_utils::Message
 * object and returns the resulting message. In generalConfig
 *
 * @return A spi_utils::Message object containing the generated setup configuration message.
 */
uint8_t AD4115::setupConfig(void) {
	
	spi_utils::Message msg = setupConfigMsg();
	
	msg.blockSize = 3;
    msg.nBlocks = 1;
	
	SPI.beginTransaction(adcSettings);
	
	for (uint8_t block = 0; block < msg.nBlocks; block++) {

        for (uint8_t db = 0; db < msg.blockSize; db++) {

            SPI.transfer(msg.msg[block * msg.blockSize + db]);
        }
    }
    SPI.endTransaction();

    return 0;
}

/**
 * @brief Generates an interface mode message for the AD4115 ADC.
 *
 * This function generates an interface mode message for the AD4115 ADC. The message includes specific values
 * for different configuration parameters related to the ADC's interface mode. The function constructs the message
 * by assigning values to the corresponding elements of the spi_utils::Message object and returns the resulting message.
 *
 * @return A spi_utils::Message object containing the generated interface mode message.
 */
spi_utils::Message AD4115::interfaceModeMsg(void) {

	spi_utils::Message msg;

	// 0 -- WEN [7]
	// 0 -- WRITE [6]
    // 000010 -- Address [0:5]
    msg.msg[0] = 0x02; // Send 0000 0010

	// Reserved [13:15] -- 000
	// ALT_SYNC [12] -- 0 (e.g. disabled)
	// Drive strength of DOUT/DRY pin [11] -- 0 (e.g. disabled)
	// Reserved [9:10] -- 00
    // DOUT_RESET [8] -- 0 (e.g. disabled)
    msg.msg[1] = 0x00; // Send 0000 0000

	// Enables continue read mode [7] -- 0 (e.g. disabled)
	// DATA_STAT [6] -- 0 (e.g. disabled)
	// Register intgrity checker [5] -- 0 (e.g. disabled)
	// Reserved [4] -- 0
	// CRC protection [2:3] -- 00 (e.g. disabled)
	// Reserved [1] -- 0
    // Change ADC to 16 bits [0] -- 0 (e.g. 24 bits)
    msg.msg[2] = 0x00; // Send 0000 0000

    return msg;
}

/**
 * @brief Performs the interface mode configuration for the AD4115 ADC.
 *
 * This function performs the interface mode configuration for the AD4115 ADC by sending the interface mode
 * message generated by the interfaceModeMsg() function via SPI communication. The function sets the block size
 * and number of blocks in the message, begins an SPI transaction, transfers the message data in blocks,
 * and ends the transaction. Finally, it sets the SYNC pin to HIGH to signal the end of the general configuration.
 *
 * @return 0 indicating successful execution.
 */
uint8_t AD4115::interfaceMode(void) {

	spi_utils::Message msg = interfaceModeMsg();

	msg.blockSize = 3;
	msg.nBlocks = 1;

	SPI.beginTransaction(adcSettings);

	for (uint8_t block = 0; block < msg.nBlocks; block++) {

        for (uint8_t db = 0; db < msg.blockSize; db++) {

            SPI.transfer(msg.msg[block * msg.blockSize + db]);
        }
    }

    //Sync set to HIGH. End of generalConfig
    digitalWrite(_adcSync, HIGH);
    
    SPI.endTransaction();

    return 0;
}

/**
 * @brief Performs the general configuration for the AD4115 ADC.
 *
 * This function performs the general configuration for the AD4115 ADC by calling the configChannel(),
 * setupConfig(), and interfaceMode() functions. It passes the specified channel, state, setup, input_1,
 * and input_2 parameters to the configChannel() function to configure the channel. It then calls the
 * setupConfig() function to perform the setup configuration and the interfaceMode() function to configure
 * the interface mode. The resulting values of the individual configuration steps are stored in db1, db2, and db3,
 * respectively. The function returns 0 indicating successful execution.
 *
 * @param channel  The channel number to configure.
 * @param state    The state (enable/disable) of the channel.
 * @param setup    The setup value for the channel.
 * @param input_1  The first input configuration for the channel.
 * @param input_2  The second input configuration for the channel.
 *
 * @return 0 indicating successful execution.
 */
uint8_t AD4115::generalConfig(uint8_t channel, uint8_t state, uint8_t setup, uint8_t input_1, uint8_t input_2) {
	
	uint8_t db1 = configChannel(channel, state, setup, input_1, input_2);
	uint8_t db2 = setupConfig();
	uint8_t db3 = interfaceMode();

	return 0;
}

/**
 * @brief Generates an ADC mode message for the AD4115 ADC.
 *
 * This function generates an ADC mode message for the AD4115 ADC. The message includes specific values for
 * different configuration parameters related to the ADC's mode of operation. The function constructs the message
 * by assigning values to the corresponding elements of the spi_utils::Message object and returns the resulting message.
 *
 * @return A spi_utils::Message object containing the generated ADC mode message.
 */
spi_utils::Message AD4115::adcModeMsg() {
	
	spi_utils::Message msg;

	// 000001 -- Address [0:5]
	// 0 -- WRITE [6]
	// 0 -- WEN [7]
    msg.msg[0] = 0x01; // Send 0000 0001 

	// REF_EN [15] -- 0 (e.g. disabled)
	// Reserved [14] -- 0
	// ON if single channel active [13] -- 0 (e.g. disabled)
	// Reserved [11:12] -- 00
    // Delay [8:10] -- 000 (e.g. 0 microsecs)
    msg.msg[1] = 0x00; // Send 0000 0000

	// Reserved [7] -- 0
	// Operating mode [4:6] -- 001 (e.g. single conversion mode)
	// ADC clock source [2:3] -- 11 kk
    // Reserved [0:1] -- 00
    msg.msg[2] = 0x1C; // Send 0001 1100

    return msg;
}

/**
 * @brief Performs the ADC mode configuration for the AD4115 ADC.
 *
 * This function performs the ADC mode configuration for the AD4115 ADC by sending the ADC mode message generated
 * by the adcModeMsg() function via SPI communication. The function sets the block size and number of blocks in
 * the message, begins an SPI transaction, transfers the message data in blocks, and ends the transaction. It uses
 * the _adcSync pin to control the synchronization of the ADC mode configuration. The function sets the _adcSync
 * pin to LOW before starting the transfer and sets it to LOW again after the transfer. Finally, it ends the SPI
 * transaction. In full_reading
 *
 * @return None.
 */
void AD4115::adcMode(void) {
	spi_utils::Message msg = adcModeMsg();

	msg.blockSize = 3;
	msg.nBlocks = 1;

	SPI.beginTransaction(adcSettings);

	for (uint8_t block = 0; block < msg.nBlocks; block++) {

        digitalWrite(_adcSync, LOW);

        for (uint8_t db = 0; db < msg.blockSize; db++) {

            SPI.transfer(msg.msg[block * msg.blockSize + db]);
        }

        //digitalWrite(_adcSync, LOW);
    }
    SPI.endTransaction();
}

/**
 * @brief Updates the channel states based on the ADC response.
 *
 * This function updates the channel states based on the response received from the ADC.
 * It sends commands to the ADC to read the state of each channel, extracts the state
 * information from the response, and updates the corresponding channel state in the _channelStates array.
 * The function uses SPI communication to transfer the commands and responses with the ADC.
 * It sets the block size and number of blocks in the spi_utils::Message object, begins an SPI transaction,
 * transfers the data in blocks, and ends the transaction. The _adcSync pin is used to control the synchronization
 * of the SPI communication. The function returns 0 indicating successful execution.
 * Function not in use under the current configuration.
 * 
 * @return 0 indicating successful execution.
 */
uint8_t AD4115::updateChannelStates(void) {

	uint8_t state_mask = 0x80; //1000 0000

	spi_utils::Message msg;

	msg.blockSize = 16;
	msg.nBlocks = 1;

	SPI.beginTransaction(adcSettings);

	for (uint8_t block = 0; block < msg.nBlocks; block++) {

        digitalWrite(_adcSync, LOW);

        for (uint8_t db = 0; db < msg.blockSize; db++) {

            SPI.transfer(0x50 + db);
    		uint8_t db1 = SPI.transfer(0x00);
    		uint8_t db2 = SPI.transfer(0x00); //irrelevant

    		uint8_t state = (state_mask & db1);

    		if (state == 0x80) {
    			_channelStates[db] = 1;
    		}
    		else {
    			_channelStates[db] = 0;
    		}
        }
        digitalWrite(_adcSync, HIGH);
    }
    SPI.endTransaction();

    return 0;	
}

/**
 * @brief Maps the decimal value to voltage.
 *
 * Only mapping to bipolar code is implemented. This function maps a decimal value to voltage using a specific formula.
 * It takes a decimal value as input, divides it by 8388608 (2^23), subtracts 1, and multiplies the result by 25.
 * The resulting value represents the voltage mapped from the decimal input. The function returns the mapped voltage
 * as a double precision value.
 *
 * @param decimal The decimal value to be mapped to voltage.
 * @return The mapped voltage as a double precision value.
 */
double AD4115::voltageMap(double decimal) {
	return ((double) ((decimal / 8388608-1) * 25));
}

/**
 * @brief Converts three bytes to a double precision integer.
 *
 * This function converts three bytes to a double precision integer by performing bitwise operations.
 * It takes three bytes (db1, db2, db3) as input and combines them into a 24-bit integer value.
 * The function performs left-shift operations on db1 and db2 to position them correctly, and then adds
 * db3 to the result. The resulting value represents the three bytes combined into a double precision integer.
 * The function returns the converted value as a double precision floating-point number.
 *
 * @param db1 The most significant byte of the three-byte value.
 * @param db2 The middle byte of the three-byte value.
 * @param db3 The least significant byte of the three-byte value.
 * @return The converted value as a double precision floating-point number.
 */
double AD4115::threeByteToInt(uint8_t db1, uint8_t db2, uint8_t db3) {
	return ( (double) ( ( ( db1 << 8 ) + db2 ) << 8 ) + db3 );
}

/**
 * @brief Divides an integer number into three bytes.
 *
 * This function divides an integer number into three bytes (db1, db2, db3) by performing bitwise operations.
 * The resulting bytes d1, db2 and db3 are assigned to _dataRead[0], _dataRead[1] and _dataRead[2], respectively.
 *
 * @param decimal The integer to be divided.
 * @return None.
 */
void AD4115::intToThreeByte(uint32_t decimal) {
	uint8_t db1 = decimal >> 16;
	uint8_t db2 = (decimal >> 8) - (db1 << 8);
	uint8_t db3 = decimal - ((db2 << 8) + (db1 << 8));

	_dataRead[0] = db1;
	_dataRead[1] = db2;
	_dataRead[2] = db3;
}

/**
 * @brief Generates a data reading message for the AD4115 ADC.
 *
 * This function generates a data reading message for the AD4115 ADC. The message is used to request data from the ADC.
 * The function assigns specific values to the elements of the spi_utils::Message object, representing the command to
 * read data from the ADC. The function then returns the generated message.
 *
 * @return A spi_utils::Message object containing the generated data reading message.
 */
spi_utils::Message AD4115::dataReadingMsg(void) {

	spi_utils::Message msg;

	msg.msg[0] = 0x44;
	msg.msg[1] = 0x00;
	msg.msg[2] = 0x00;
	msg.msg[3] = 0x00;

	return msg;
}

/**
 * @brief Performs data reading from the AD4115 ADC.
 *
 * This function performs data reading from the AD4115 ADC by sending a data reading message generated by the
 * dataReadingMsg() function via SPI communication. The function sets the block size and number of blocks in the
 * message, begins an SPI transaction, transfers the message data in blocks, and ends the transaction.
 * It uses the SPI.transfer() function to send and receive data from the ADC. The first byte of the message is sent
 * without storing the received value, and the following three bytes are received and stored in the _dataRead array.
 * The function iterates through the message blocks and data bytes, excluding the first byte. Finally, it ends the SPI
 * transaction.
 *
 * @return None.
 */
void AD4115::dataReading(void) {

	spi_utils::Message msg = dataReadingMsg();

	msg.blockSize = 4;
	msg.nBlocks = 1;

	SPI.beginTransaction(adcSettings);

	for (uint8_t block = 0; block < msg.nBlocks; block++) {

        for (uint8_t db = 0; db < msg.blockSize; db++) {

        	if (db == 0) {
        		SPI.transfer(msg.msg[block * msg.blockSize + db]);
        	}
            else {
            	_dataRead[db - 1] = SPI.transfer(msg.msg[block * msg.blockSize + db]);
            } 
        }
    }
    SPI.endTransaction();
}

/**
 * @brief Performs a full reading from the AD4115 ADC.
 *
 * This function performs a full reading from the AD4115 ADC by executing the necessary steps. First, it sets the ADC mode
 * using the `adcMode()` function. Then, it iterates through all 16 channels and performs the following steps for each
 * active channel:
 *   1. Waits for the DRDY signal using the `waitDrdy()` function.
 *   2. Reads the data from the ADC using the `dataReading()` function.
 *   3. Converts the read data to a decimal value using the `threeByteToInt()` function and stores it in the `_channelDecimals` array.
 *   4. Maps the decimal value to voltage using the `voltageMap()` function and stores it in the `_channelVoltages` array.
 * After processing all active channels, the function sets the _adcSync pin to HIGH. Finally, it iterates through all active
 * channels again and prints the channel number and corresponding voltage to the serial monitor. The function returns 0
 * indicating successful execution.
 *
 * @return 0 indicating successful execution.
 */
double AD4115::fullReading(void) {
	adcMode();
	for (int i = 0; i < 16; i++) {
		if (_channelStates[i] == 1) {

			waitDrdy();

			dataReading();

			_channelDecimals[i] = threeByteToInt(_dataRead[0], _dataRead[1], _dataRead[2]);
    		_channelVoltages[i] = voltageMap(_channelDecimals[i]); 
		}
	}

	digitalWrite(_adcSync, HIGH);

	for (int i = 0; i < 16; i++) {
		if (_channelStates[i] == 1) {
			Serial.print("Channel ");
			Serial.print(i);
			Serial.print(":");
			Serial.print(_channelVoltages[i],6);
			Serial.println("V");
		}
	}
	return 0;
}

/**
 * @brief Performs a buffer ramp full reading from the AD4115 ADC.
 *
 * This function performs a buffer ramp full reading from the AD4115 ADC by executing the necessary steps. First, it sets the ADC mode
 * using the `adcMode()` function. Then, it iterates through all 16 channels and performs the following steps for each
 * active channel:
 *   1. Waits for the DRDY signal using the `waitDrdy()` function.
 *   2. Reads the data from the ADC using the `dataReading()` function.
 *   3. Converts the read data to a decimal value using the `threeByteToInt()` function and stores it in the `_channelDecimals` array.
 *   4. Maps the decimal value to voltage using the `voltageMap()` function and stores it in the `_channelVoltages` array.
 * After processing all active channels, the function sets the `_adcSync` pin to HIGH. Finally, it iterates through all active
 * channels again and prints the corresponding voltages in binary format to using the `Serial.write()` function.
 * The function returns 0 indicating successful execution.
 *
 * @return 0 indicating successful execution.
 */
double AD4115::bufferRampFullReading(void) {
	adcMode();

	for (int i = 0; i < 16; i++) {
		if (_channelStates[i] == 1) {
			waitDrdy();
			dataReading();

			_channelDecimals[i] = threeByteToInt(_dataRead[0], _dataRead[1], _dataRead[2]);
    		_channelVoltages[i] = voltageMap(_channelDecimals[i]); 
		}
	}

	digitalWrite(_adcSync, HIGH);

	for (int i = 0; i < 16; i++) {
		if (_channelStates[i] == 1) {
			intToThreeByte(_channelDecimals[i]);
			Serial.write(_dataRead[0]);
			Serial.write(_dataRead[1]);
			Serial.write(_dataRead[2]);
		}
	}
	return 0;
}


/**
 * @brief Performs a test to read and print the configuration of the ADC channels.
 *
 * This function performs a test to read and print the configuration of the ADC channels. It reads the channel registers
 * by sending specific message bits to the communications register of the ADC. The function iterates through all 16 channels
 * and performs the following steps for each channel:
 *   1. Constructs the message bits by combining the communication register bits and the channel index.
 *   2. Sends the message bits to the ADC using the `SPI.transfer()` function.
 *   3. Reads the data bytes of the channel from the ADC and combines them to obtain a double precision value.
 *   4. Prints the channel index and the read data bytes, as well as the combined value, to the serial monitor.
 * After processing all channels, the function sets the `_adcSync` pin to HIGH and ends the SPI transaction.
 * The function does not return a value. Returns the bits that configured each channel. Check data sheet to read this data.
 */ 
uint8_t AD4115::configChannelsTest(void) {
	
	uint16_t msg_bits;
	uint8_t db1;
	uint8_t db2;
	double db_final;
	
	//Message bits to communications register to read a register
	uint32_t comm_reg_bits = 5;
	
	SPI.beginTransaction(adcSettings);
	digitalWrite(_adcSync, LOW);
	Serial.println("Reading channels registers\n");
	for (uint8_t i = 0; i < 16; i++) {
		

		//Message bits to read channel i
		msg_bits = (comm_reg_bits << 4) + i;
		Serial.print("msg_bits");
		Serial.println(msg_bits);
		SPI.transfer(msg_bits);
		db1 = SPI.transfer(0X00);
		db2 = SPI.transfer(0x00);

		db_final = (db1 << 8) + db2;

		//Test prints
		Serial.print("Reading channel ");
		Serial.print(i);
		Serial.print(",  ");
		Serial.print("Data: ");
		Serial.print(db1);
		Serial.print(", ");				
		Serial.print(db2);
		Serial.print(", ");				
		Serial.println(db_final);

	}
	digitalWrite(_adcSync, HIGH);
	SPI.endTransaction();

}	