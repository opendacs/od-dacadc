#ifndef UTILS_H
#define UTILS_H
#include <SPI.h>
#include <stdint.h>
#include <WString.h>
using namespace std;

typedef unsigned char byte;


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

