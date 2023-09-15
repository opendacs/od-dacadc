#ifndef UTILS_H
#define UTILS_H
#include <SPI.h>
#include <stdint.h>
#include <WString.h>
using namespace std;

typedef unsigned char byte;

/**
 * @namespace spi_utils
 * @brief Namespace containing utility functions and structures for SPI communication.
 *
 * The spi_utils namespace provides a structure called Message, which represents a message to be sent via SPI.
 * The Message structure contains an array 'msg' to hold the message bytes, as well as 'blockSize' and 'nBlocks' variables
 * to specify the size and number of blocks in the message.
 *
 * This namespace is used in several functions throughout the code. For example, in the AD4115 class, the `dataReadingMsg`
 * function returns an instance of spi_utils::Message to define the message for data reading. Similarly, the AD5791 class
 * uses spi_utils::Message in functions like `setVoltageMsg` and `readDacMsg` to construct messages for setting voltage
 * and reading the DAC value.
 *
 * While not currently used, the commented-out function 'data_transfer' demonstrates a potential use of the Message structure.
 * It could be used to perform the data transfer over SPI by iterating over the blocks and transferring the data bytes using
 * the SPI.transfer function. However, this function is currently not utilized in the provided code.
 *
 * Overall, the spi_utils namespace encapsulates utility functions and structures for SPI communication, which are used in
 * various parts of the code to construct and transfer messages over SPI.
 */
namespace spi_utils
{
    struct Message {
        ///
        /// Size of msg. (blockSize*nBlocks <= kDataLen)
        ///
        static const uint8_t kDataLen = 64;
        ///
        ///
        /// Message to be sent via SPI. Each element represents a byte. This message is
        /// separated could be divided in blocks.
        ///
        byte msg[kDataLen];

        byte data[kDataLen];
        ///
        ///
        /// The size in bytes of the registers to be written.
        /// It can be also be thought of as the number of bytes to be sent before setting
        /// the sync_pin_ to HIGH. (blockSize*nBlocks <= kDataLen)
        ///
        uint8_t blockSize;
        ///
        ///
        /// The number of blocks. Each block starts with a sync_pin_ to LOW and ends with
        /// a sync_pin_ to HIGH. (blockSize*nBlocks <= kDataLen)
        ///
        uint8_t nBlocks;

        void transfer(SPISettings settings, uint8_t syncPin, bool continue_tx){
            SPI.beginTransaction(settings);

            for (uint8_t block = 0; block < nBlocks; block++) {

                digitalWrite(syncPin, LOW);

                for (uint8_t db = 0; db < blockSize; db++) {

                    SPI.transfer(msg[block * blockSize + db]);
                }
                if (!continue_tx) {
                    digitalWrite(syncPin, HIGH);
                }
            }
            SPI.endTransaction();
        }

        void errorMessage(){
            for (size_t i = 0; i < kDataLen; i++) {
                msg[i] = 0xFF;
            }    
        }
      };

    //   uint8_t data_transfer(uint8_t data[], uint8_t blockSize, uint8_t nBlocks, uint8_t sync_pin) {
        
    //     spi_utils::Message msg = data;

    //     msg.blockSize = blockSize;
    //     msg.nBlocks = nBlocks;

    //     for (uint8_t block = 0; block < msg.nBlocks; block++) {

    //         digitalWrite(sync_pin, LOW);

    //         for (uint8_t db = 0; db < msg.blockSize; db++) {

    //             SPI.transfer(msg.msg[block * msg.blockSize + db]);
    //         }
    //         digitalWrite(sync_pin, HIGH);
    //     }

    //     return 0;
    // }
}

/**
 * @namespace interface_utils
 * @brief Namespace containing utility functions for serial interface parsing.
 *
 * The interface_utils namespace provides utility functions for parsing and storing values separated by "," or ":" from
 * a serial message. The main function in this namespace is `querySerial`, which takes a serial message as input and parses
 * it into separate elements stored in the `cmd` array. It returns the number of values parsed.
 *
 * For example, if the serial message is "SET, 1: 4.2", the `cmd` array will be {SET, 1, 4.2}, and the function will return 3.
 *
 * This namespace is used in the main `loop` function to process incoming serial commands. It extracts the command and its
 * parameters from the serial message and calls the appropriate router function to handle the command.
 *
 * Overall, the interface_utils namespace encapsulates utility functions for parsing serial interface messages and
 * extracting individual values for further processing.
 */
namespace interface_utils {
    ///
    /// Parses and stores values separated by "," or ":" from serial message to cmd[]
    /// Returns number of values
    /// Example: if Serial message "SET, 1: 4.2"
    /// cmd[] will be {SET, 1, 4.2} and returns 3
    ///
    static uint8_t querySerial(String cmd[]) {

        char received;

        String cmdElement = "";

        uint8_t cmdSize = 0;

        while (received != '\r') {

            if(Serial.available()) {

                received = Serial.read();

                if (received == '\n' || received == ' ') {}

                else if (received == ',' || received == '\r' || received == ':') {

                    cmd[cmdSize] = cmdElement;

                    cmdElement = "";

                    ++cmdSize;
                }

                else {

                    cmdElement += received;
                }
            }
        }
            return cmdSize;
    }
}

#endif // UTILS_H

