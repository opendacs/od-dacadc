#ifndef UTILS_H
#define UTILS_H
#include <SPI.h>
#include <stdint.h>
#include <WString.h>
using namespace std;

typedef unsigned char byte;

namespace spi_utils
{
    struct Message {
        ///
        /// Size of msg. (block_size*n_blocks <= kdata_len_)
        ///
        static const uint8_t kdata_len_ = 10;
        ///
        ///
        /// Message to be sent via SPI. Each element represents a byte. This message is
        /// separated could be divided in blocks.
        ///
        byte msg[kdata_len_];
        ///
        ///
        /// The size in bytes of the registers to be written.
        /// It can be also be thought of as the number of bytes to be sent before setting
        /// the sync_pin_ to HIGH. (block_size*n_blocks <= kdata_len_)
        ///
        uint8_t block_size;
        ///
        ///
        /// The number of blocks. Each block starts with a sync_pin_ to LOW and ends with
        /// a sync_pin_ to HIGH. (block_size*n_blocks <= kdata_len_)
        ///
        uint8_t n_blocks;
      };
}

namespace interface_utils {
    ///
    /// Parses and stores values separated by "," or ":" from serial message to cmd[]
    /// Returns number of values
    /// Example: if Serial message "SET, 1: 4.2"
    /// cmd[] will be {SET, 1, 4.2} and returns 3
    ///
    static uint8_t query_serial(String cmd[]) {

        char received;

        String cmd_element = "";

        uint8_t cmd_size = 0;

        while (received != '\r') {

            if(Serial.available()) {

                received = Serial.read();

                if (received == '\n' || received == ' ') {}

                else if (received == ',' || received == '\r' || received == ':') {

                    cmd[cmd_size] = cmd_element;

                    cmd_element = "";

                    ++cmd_size;
                }

                else {

                    cmd_element += received;
                }
            }
        }
            return cmd_size;
    }
}

#endif // UTILS_H

