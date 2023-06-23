#include "../include/ad5791.h"
#include <stdint.h>
#include <SPI.h>

/**
 * @brief Updates the analog outputs of the AD5791 DAC.
 *
 * This function updates the analog outputs of the AD5791 DAC. It sets the LDAC (Load DAC) pin to LOW and then to HIGH
 * in order to update the outputs. The function is responsible for triggering the update process of the DAC. After
 * setting the LDAC pin to HIGH, the DAC outputs the new analog values. This function assumes that the DAC has been
 * previously configured and the analog values have been set. It does not take any input parameters or return any values.
 */
void AD5791::updateAnalogOutputs(void) {
    digitalWrite(LDAC, LOW);
    digitalWrite(LDAC, HIGH);
}

/**
 * @brief Converts a three-byte message to voltage for the AD5791 DAC.
 *
 * This function converts a three-byte message received from the AD5791 DAC to a corresponding voltage value. It takes a
 * `spi_utils::Message` object as input, which contains the three bytes of the message. The function extracts the three
 * bytes from the message and combines them to form a two's complement decimal value using the `threeByteToInt()` function.
 * It then calculates the voltage based on the decimal value using a specific formula for the AD5791 DAC. If the decimal
 * value is less than or equal to 524287, the function calculates the voltage as the decimal value multiplied by the
 * DAC full scale divided by 524287. Otherwise, if the decimal value is greater than 524287, the function calculates
 * the voltage as the negation of (1048576 minus the decimal value) multiplied by the DAC full scale divided by 524288.
 * The calculated voltage is returned by the function as a `double` value.
 *
 * @param message The `spi_utils::Message` object containing the three-byte message.
 * @return The corresponding voltage value based on the received message.
 */
double AD5791::bytesToVoltage(spi_utils::Message message) {

    byte byte1 = message.msg[0];
    byte byte2 = message.msg[1];
    byte byte3 = message.msg[2];

    // The conversion below is for two's complement
    uint32_t decimal = threeByteToInt(byte1, byte2, byte3);
    double voltage;
    if (decimal <= 524287) {
        voltage = decimal * DAC_FULL_SCALE / 524287;
    }
    else {
        voltage = -(1048576 - decimal) * DAC_FULL_SCALE / 524288;
    }
    return voltage;
}

/**
 * @brief Generates a SPI message to set the voltage on the AD5791 DAC.
 *
 * This function generates a SPI message to set the desired voltage on the AD5791 DAC. It takes a voltage value as input and
 * returns a `spi_utils::Message` object that represents the SPI message. The function performs the following steps:
 *   1. Calculates the two's complement decimal value based on the input voltage using a specific formula for the AD5791 DAC.
 *   2. Constructs a `spi_utils::Message` object to store the SPI message.
 *   3. Sets the message bytes according to the calculated decimal value, considering the byte order specified in the datasheet.
 *      - The first byte is constructed from the most significant 8 bits of the decimal value, including a control bit to write to the DAC register.
 *      - The second byte is constructed from the next 8 bits of the decimal value.
 *      - The third byte is constructed from the least significant 8 bits of the decimal value.
 *   4. Returns the generated `spi_utils::Message` object.
 *
 * @param voltage The desired voltage to be set on the AD5791 DAC.
 * @return A `spi_utils::Message` object representing the SPI message to set the voltage.
 */
spi_utils::Message AD5791::setVoltageMsg(double voltage) {

    uint32_t decimal;
    spi_utils::Message msg;

    // The conversion below is for two's complement
    if (voltage < 0) {
        decimal = voltage * 524288 / DAC_FULL_SCALE + 1048576;
    }
    else {
        decimal = voltage * 524287 / DAC_FULL_SCALE;
    }

    // Check datasheet for details. 1000 0001 0000 0010 0001 0000 0010
    msg.msg[0] = (byte)((decimal >> 16) | 16);  // Writes to dac register
    msg.msg[1] = (byte)((decimal >> 8) & 255);  // Writes first byte
    msg.msg[2] = (byte)(decimal & 255);  // Writes second byte
    Serial.println("setVoltageMsg debugging: ");
    Serial.println(msg.msg[0]);
    Serial.println(msg.msg[1]);
    Serial.println(msg.msg[2]);
    return msg;
}

/**
 * @brief Sets the voltage on the specified channel of the AD5791 DAC.
 *
 * This function sets the voltage on the specified channel of the AD5791 DAC. It takes the channel number, desired voltage,
 * and an optional flag to update the analog outputs as input. The function performs the following steps:
 *   1. Begins a SPI transaction with the DAC settings.
 *   2. Generates a SPI message using the `setVoltageMsg` function to set the desired voltage.
 *   3. Sets the block size and number of blocks in the message to 3 and 1, respectively.
 *   4. Checks if the desired voltage is within the valid range. If it exceeds the range, it prints an error message and returns 999.
 *   5. If the voltage is within the valid range:
 *      - For each block in the message, it brings the corresponding DAC sync pin LOW.
 *      - Transfers the message bytes using SPI.transfer to send the voltage data.
 *      - Sets the corresponding DAC sync pin HIGH to complete the transfer.
 *   6. If the `updateOutputs` flag is true, it calls the `updateAnalogOutputs` function to update the analog outputs.
 *   7. Calculates the updated voltage based on the transferred message using the `bytesToVoltage` function and stores it in the `vReadings` array.
 *   8. Returns the updated voltage.
 *
 * @param channel The channel number of the AD5791 DAC to set the voltage on.
 * @param voltage The desired voltage to be set on the specified channel.
 * @param updateOutputs Flag indicating whether to update the analog outputs.
 * @return The updated voltage on the specified channel.
 *         If the desired voltage exceeds the valid range, it returns 999.
 */
double AD5791::setVoltage(uint8_t channel, double voltage, bool updateOutputs) {

    SPI.beginTransaction(dacSettings);

    spi_utils::Message msg = setVoltageMsg(voltage);
    msg.blockSize = 3;
    msg.nBlocks = 1;



    if (voltage < -1 * DAC_FULL_SCALE || voltage > DAC_FULL_SCALE) {
        Serial.println("VOLTAGE OVERRANGE");
        return 999;
    }

    else {

        for (uint8_t block = 0; block < msg.nBlocks; block++) {
            digitalWrite(dacSyncPins[channel], LOW);
            
            for (uint8_t db = 0; db < msg.blockSize; db++) {
                SPI.transfer(msg.msg[block * msg.blockSize + db]);
            }
            digitalWrite(dacSyncPins[channel], HIGH);
        }

        if (updateOutputs) {updateAnalogOutputs();}

        // Updated voltage may be different than voltage parameter because of
        // resolution
        vReadings[channel] = bytesToVoltage(msg);
        Serial.println("vReadings[channel]");
        Serial.println(vReadings[channel]);
        return bytesToVoltage(msg);    
    }
}

/**
 * @brief Converts an integer value to three separate bytes.
 *
 * This function converts the given integer value to three separate bytes. It takes the integer value as input and uses
 * pointer parameters to store the resulting bytes. The function performs the following steps:
 *   1. Converts the integer value to three bytes by shifting and masking operations.
 *      - The most significant byte is obtained by right-shifting the integer value by 16 bits and ORing it with 16 to set a control bit.
 *      - The middle byte is obtained by right-shifting the integer value by 8 bits and masking it with 255 (0xFF) to get the 8 least significant bits.
 *      - The least significant byte is obtained by masking the integer value with 255 (0xFF) to get the 8 least significant bits.
 *   2. Stores the three resulting bytes in the memory locations pointed to by the DB1, DB2, and DB3 pointers.
 *
 * @param decimal The integer value to be converted to three bytes.
 * @param DB1 Pointer to a byte variable to store the most significant byte.
 * @param DB2 Pointer to a byte variable to store the middle byte.
 * @param DB3 Pointer to a byte variable to store the least significant byte.
 */
uint8_t AD5791::intToThreeBytes(int decimal, byte* DB1, byte* DB2, byte* DB3) {

    *DB1 = (byte)((decimal >> 16) | 16);
    *DB2 = (byte)((decimal >> 8) & 255);
    *DB3 = (byte)(decimal & 255);
}

/**
 * @brief Constructor for the AD5791 DAC class.
 *
 * This constructor initializes an instance of the AD5791 DAC class. It takes an array of sync pins for each channel
 * and an LDAC pin as input. The function performs the following steps:
 *   1. Assigns the LDAC pin value to the class member variable LDAC.
 *   2. Copies the sync pin values from the input array to the class member array dacSyncPins, channel by channel.
 *
 * @param syncPins An array of sync pins for each channel of the AD5791 DAC.
 * @param ldacPin The LDAC pin of the AD5791 DAC.
 */
AD5791::AD5791(uint8_t syncPins[nChannels], uint8_t ldacPin) {

    LDAC = ldacPin;

    for (int i = 0; i < nChannels; ++i) {
        dacSyncPins[i] = syncPins[i];
    }
}

/**
 * @brief Generates an initialization message for the AD5791 DAC.
 *
 * This function generates an initialization message for the AD5791 DAC. It creates a spi_utils::Message object and sets
 * the message bytes to specific values. The generated message is used to initialize the DAC. The function performs the following steps:
 *   1. Creates a spi_utils::Message object to store the initialization message.
 *   2. Sets the first byte of the message to 0x20, indicating the specific command or address.
 *   3. Sets the second byte of the message to 0x00, representing a specific data value or parameter.
 *   4. Sets the third byte of the message to 0x02, representing another data value or parameter.
 *   5. Returns the generated message to be used for initialization.
 *
 * @return A spi_utils::Message object containing the initialization message for the AD5791 DAC.
 */
spi_utils::Message AD5791::initializeMsg(void) {

    spi_utils::Message msg;
    msg.msg[0] = 0x20;
    msg.msg[1] = 0x00;
    msg.msg[2] = 0x02;
    return msg;
}

/**
 * @brief Initializes the AD5791 DAC.
 *
 * This function initializes the AD5791 DAC by sending an initialization message to each DAC channel. It performs the following steps:
 *   1. Begins the SPI transaction with the DAC settings.
 *   2. Generates an initialization message using the initializeMsg() function.
 *   3. Sets the block size and number of blocks in the message.
 *   4. Iterates over each DAC channel:
 *      - Lowers the sync pin of the current DAC channel.
 *      - Transfers the bytes of the initialization message to the DAC.
 *      - Raises the sync pin of the current DAC channel.
 *   5. Ends the SPI transaction.
 *   6. Returns 0 to indicate successful initialization.
 *
 * @return An integer value of 0 indicating successful initialization of the AD5791 DAC.
 */
uint8_t AD5791::initialize(void) {

    SPI.beginTransaction(dacSettings);
    spi_utils::Message msg = initializeMsg();
    msg.blockSize = 3;
    msg.nBlocks = 1;

    for (uint8_t dacPin = 0; dacPin < nChannels; dacPin++) {

        for (uint8_t block = 0; block < msg.nBlocks; block++) {
            digitalWrite(dacSyncPins[dacPin], LOW);

            for (uint8_t db = 0; db < msg.blockSize; db++) {
                SPI.transfer(msg.msg[block * msg.blockSize + db]);

            }
            digitalWrite(dacSyncPins[dacPin], HIGH);
        }

    }
    return 0;
}


/**
 * @brief Initializes the AD5791 DAC and sets up the required pins and SPI.
 *
 * This function initializes the AD5791 DAC by performing the following steps:
 *   1. Iterates over each DAC channel:
 *      - Sets the pin mode of the sync pin to OUTPUT.
 *      - Sets the initial value of the sync pin to HIGH.
 *   2. Sets the pin mode of the LDAC pin to OUTPUT.
 *   3. Sets the initial value of the LDAC pin to HIGH.
 *   4. Initializes and configures the SPI communication.
 *
 * @note This function assumes that the `dacSyncPins` array and `LDAC` pin have been properly defined and configured prior to calling this function.
 */
uint8_t AD5791::begin(void) {

    for (int dac = 0; dac < 3; ++dac) {

        // Setting pin modes
        pinMode(dacSyncPins[dac], OUTPUT);

        // Setting pin values
        digitalWrite(dacSyncPins[dac], HIGH);
    }

    // Setting LDAC mode
    pinMode(LDAC, OUTPUT);

    // Setting LDAC value
    digitalWrite(LDAC, HIGH);

    // Initializing and configuring SPI
    SPI.begin();
}

/**
 * @brief Generates a SPI message with three null bytes.
 *
 * This function generates a SPI message containing three null bytes. It performs the following steps:
 *   1. Creates an instance of the `spi_utils::Message` struct named `msg2`.
 *   2. Assigns the value 0x00 to the first byte of the message, representing the command byte.
 *   3. Assigns the value 0x00 to the second byte of the message.
 *   4. Assigns the value 0x00 to the third byte of the message.
 *   5. Returns the generated message `msg2`.
 *
 * @return A `spi_utils::Message` object containing three null bytes.
 */
spi_utils::Message AD5791::threeNullBytesMsg(void) {

    spi_utils::Message msg2;

    msg2.msg[0] = 0x00; //Command byte
    msg2.msg[1] = 0x00;
    msg2.msg[2] = 0x00;
    return msg2;
}

/**
 * @brief Reads the voltage value from the specified channel.
 *
 * This function reads and returns the voltage value from the specified channel of the AD5791 DAC. It performs the following steps:
 *   1. Checks if the channel is within the valid range (0 to 4).
 *      - If the channel is outside the valid range, it prints an error message and returns 0.
 *   2. If the channel is valid, it returns the corresponding voltage reading from the `vReadings` array.
 *
 * @param channel The channel number from which to read the voltage (0 to 4).
 * @return The voltage value from the specified channel, or 0 if the channel is invalid.
 */
double AD5791::readVoltage(uint8_t channel) {

    if (channel > 4 || channel < 0) {
        Serial.println("Invalid channel");
        return 0;
    }
    else {return vReadings[channel];}
}

/**
 * @brief Generates a SPI message to read the DAC value.
 *
 * This function generates a SPI message to read the DAC value from the AD5791 DAC. It performs the following steps:
 *   1. Creates an instance of the `spi_utils::Message` struct named `msg`.
 *   2. Assigns the value 0x90 to the first byte of the message, representing the command byte for reading the DAC value.
 *   3. Assigns the value 0x00 to the second byte of the message.
 *   4. Assigns the value 0x00 to the third byte of the message.
 *   5. Returns the generated message `msg`.
 *
 * @return A `spi_utils::Message` object for reading the DAC value.
 */
spi_utils::Message AD5791::readDacMsg(void) {

    spi_utils::Message msg;

    msg.msg[0] = 0x90; //Command byte
    msg.msg[1] = 0x00;
    msg.msg[2] = 0x00;
    return msg;
}

/**
 * @brief Reads the DAC value from the specified channel.
 *
 * This function reads the DAC value from the specified channel of the AD5791 DAC. It performs the following steps:
 *   1. Creates an instance of the `spi_utils::Message` struct named `msg` using the `readDacMsg` function.
 *   2. Sets the `blockSize` and `nBlocks` fields of `msg` to 3 and 1, respectively.
 *   3. Iterates over the `nBlocks` and transfers the bytes of `msg` over SPI, using the appropriate sync pin for the channel.
 *   4. Delays for a short period of time (1 microsecond).
 *   5. Creates an array `data` to store the received bytes.
 *   6. Creates another instance of the `spi_utils::Message` struct named `msg2` using the `threeNullBytesMsg` function.
 *   7. Sets the `blockSize` and `nBlocks` fields of `msg2` to 3 and 1, respectively.
 *   8. Iterates over the `nBlocks` and transfers the bytes of `msg2` over SPI, storing the received bytes in `data`.
 *   9. Converts the received bytes in `data` to a voltage value using the `threeByteToVoltage` function.
 *   10. Returns the voltage value.
 *
 * @param channel The channel number from which to read the DAC value (0 to 2).
 * @return The DAC value as a voltage.
 */
double AD5791::readDac(uint8_t channel) {
    spi_utils::Message msg = readDacMsg();
    msg.blockSize = 3;
    msg.nBlocks = 1;

    for (uint8_t block = 0; block < msg.nBlocks; block++) {

        digitalWrite(dacSyncPins[channel], LOW);

        for (uint8_t db = 0; db < msg.blockSize; db++) {

            SPI.transfer(msg.msg[block * msg.blockSize + db]);
        }

<<<<<<< Updated upstream
        digitalWrite(dac_sync_pins[channel], HIGH);

=======
        digitalWrite(dacSyncPins[channel], HIGH);
>>>>>>> Stashed changes
    }

    delayMicroseconds(1);

    uint8_t data[3];
    spi_utils::Message msg2 = threeNullBytesMsg();
    msg2.blockSize = 3;
    msg2.nBlocks = 1;

    for (uint8_t block = 0; block < msg2.nBlocks; block++) {
        digitalWrite(dacSyncPins[channel], LOW);

        for (uint8_t db = 0; db < msg2.blockSize; db++) {
            data[db] = SPI.transfer(msg2.msg[block * msg2.blockSize + db]);       
        }
        digitalWrite(dacSyncPins[channel], HIGH);

    }

    double voltage = threeByteToVoltage(data[0], data[1], data[2]);
    Serial.println("data 0, 1, 2");
    Serial.println(data[0]);
    Serial.println(data[1]);
    Serial.println(data[2]);
    return(voltage);

}

/**
 * @brief Converts three bytes to a 32-bit integer value.
 *
 * This function takes three bytes as input and combines them to form a 32-bit integer value.
 * It performs the following steps:
 *   1. Masks the first byte (`DB1`) with `15` (binary: 00001111) to extract the lower 4 bits.
 *   2. Shifts the extracted bits 8 positions to the left.
 *   3. Combines the shifted bits with the second byte (`DB2`) using a bitwise OR operation.
 *   4. Shifts the combined value 8 positions to the left.
 *   5. Combines the shifted value with the third byte (`DB3`) using a bitwise OR operation.
 *   6. Returns the resulting 32-bit integer value.
 *
 * @param DB1 The first byte.
 * @param DB2 The second byte.
 * @param DB3 The third byte.
 * @return The combined 32-bit integer value.
 */
uint32_t AD5791::threeByteToInt(uint8_t DB1, uint8_t DB2, uint8_t DB3) {
    return ((uint32_t)(((((DB1 & 15) << 8) | DB2) << 8) | DB3));
}

/**
 * @brief Converts three bytes to a voltage value.
 *
 * This function takes three bytes as input and converts them to a voltage value based on the DAC's full scale range.
 * It performs the following steps:
 *   1. Calls the `threeByteToInt()` function to obtain a 32-bit integer value from the three bytes.
 *   2. Checks if the obtained decimal value is less than or equal to 524287.
 *   3. If true, calculates the voltage as the decimal value multiplied by the DAC's full scale divided by 524287.
 *   4. If false, calculates the voltage as the negative value of (1048576 minus the decimal value) multiplied by the DAC's full scale divided by 524288.
 *   5. Returns the calculated voltage value.
 *
 * @param DB1 The first byte.
 * @param DB2 The second byte.
 * @param DB3 The third byte.
 * @return The voltage value based on the three input bytes and the DAC's full scale range.
 */
double AD5791::threeByteToVoltage(uint8_t DB1, uint8_t DB2, uint8_t DB3) {

    double voltage;
    uint32_t decimal = threeByteToInt(DB1, DB2, DB3);

    if (decimal <= 524287) {
        voltage = decimal * DAC_FULL_SCALE / 524287;
    }
    else {
        voltage = -(1048576 - decimal) * DAC_FULL_SCALE / 524288;
    }
    return voltage;
}


