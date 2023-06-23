#include "../include/ad5791.h"
#include <stdint.h>
#include <SPI.h>

void AD5791::UpdateAnalogOutputs(void) {
    digitalWrite(LDAC, LOW);
    digitalWrite(LDAC, HIGH);
}

double AD5791::BytesToVoltage(spi_utils::Message message) {

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

spi_utils::Message AD5791::SetVoltage_Msg(double voltage) {

    uint32_t decimal;
    spi_utils::Message msg;

    // The conversion below is for two's complement
    if (voltage < 0) {
        decimal = voltage * 524288 / DAC_FULL_SCALE + 1048576;
    }
    else {
        decimal = voltage * 524287 / DAC_FULL_SCALE;
    }

    // Check datasheet for details
    msg.msg[0] = (byte)((decimal >> 16) | 16);  // Writes to dac register
    msg.msg[1] = (byte)((decimal >> 8) & 255);  // Writes first byte
    msg.msg[2] = (byte)(decimal & 255);  // Writes second byte
    return msg;
}

double AD5791::SetVoltage(uint8_t channel, double voltage, bool update_outputs) {

    SPI.beginTransaction(dacSettings);

    spi_utils::Message msg = SetVoltage_Msg(voltage);
    msg.block_size = 3;
    msg.n_blocks = 1;



    if (voltage < -1 * DAC_FULL_SCALE || voltage > DAC_FULL_SCALE) {
        Serial.println("VOLTAGE OVERRANGE");
        return 999;
    }

    else {

        for (uint8_t block = 0; block < msg.n_blocks; block++) {
            digitalWrite(dac_sync_pins[channel], LOW);
            
            for (uint8_t db = 0; db < msg.block_size; db++) {
                SPI.transfer(msg.msg[block * msg.block_size + db]);
            }
            digitalWrite(dac_sync_pins[channel], HIGH);
        }

        UpdateAnalogOutputs();

        // Updated voltage may be different than voltage parameter because of
        // resolution
        return BytesToVoltage(msg);    
    }
}

uint8_t AD5791::intToThreeBytes(int decimal, byte* DB1, byte* DB2, byte* DB3) {

    *DB1 = (byte)((decimal >> 16) | 16);
    *DB2 = (byte)((decimal >> 8) & 255);
    *DB3 = (byte)(decimal & 255);
}

AD5791::AD5791(uint8_t sync_pins[n_channels], uint8_t ldac_pin) {

    LDAC = ldac_pin;

    for (int i = 0; i < n_channels; ++i) {
        dac_sync_pins[i] = sync_pins[i];
    }
}

spi_utils::Message AD5791::Initialize_Msg(void) {

    spi_utils::Message msg;
    msg.msg[0] = 0x20;
    msg.msg[1] = 0x00;
    msg.msg[2] = 0x02;
    return msg;
}

uint8_t AD5791::Initialize(void) {

    SPI.beginTransaction(dacSettings);
    spi_utils::Message msg = Initialize_Msg();
    msg.block_size = 3;
    msg.n_blocks = 1;

    for (uint8_t dacPin = 0; dacPin < n_channels; dacPin++) {

        for (uint8_t block = 0; block < msg.n_blocks; block++) {
            digitalWrite(dac_sync_pins[dacPin], LOW);

            for (uint8_t db = 0; db < msg.block_size; db++) {
                SPI.transfer(msg.msg[block * msg.block_size + db]);

            }
            digitalWrite(dac_sync_pins[dacPin], HIGH);
        }

    }
    return 0;
}

//Configures SPI pins and initializes SPI
uint8_t AD5791::Begin(void) {

    for (int dac = 0; dac < 3; ++dac) {

        // Setting pin modes
        pinMode(dac_sync_pins[dac], OUTPUT);

        // Setting pin values
        digitalWrite(dac_sync_pins[dac], HIGH);
    }

    // Setting LDAC mode
    pinMode(LDAC, OUTPUT);

    // Setting LDAC value
    digitalWrite(LDAC, HIGH);

    // Initializing and configuring SPI
    SPI.begin();
}

spi_utils::Message AD5791::threeNullBytes_Msg(void) {

    spi_utils::Message msg2;

    msg2.msg[0] = 0x00; //Command byte
    msg2.msg[1] = 0x00;
    msg2.msg[2] = 0x00;
    return msg2;
}

spi_utils::Message AD5791::ReadDac_Msg(void) {

    spi_utils::Message msg;

    msg.msg[0] = 0x90; //Command byte
    msg.msg[1] = 0x00;
    msg.msg[2] = 0x00;
    return msg;
}

double AD5791::readDAC(uint8_t channel) {
    spi_utils::Message msg = ReadDac_Msg();
    msg.block_size = 3;
    msg.n_blocks = 1;

    for (uint8_t block = 0; block < msg.n_blocks; block++) {

        digitalWrite(dac_sync_pins[channel], LOW);

        for (uint8_t db = 0; db < msg.block_size; db++) {

            SPI.transfer(msg.msg[block * msg.block_size + db]);
        }

        digitalWrite(dac_sync_pins[channel], HIGH);

    }

    delayMicroseconds(1);

    uint8_t data[3];
    spi_utils::Message msg2 = threeNullBytes_Msg();
    msg2.block_size = 3;
    msg2.n_blocks = 1;

    for (uint8_t block = 0; block < msg2.n_blocks; block++) {
        digitalWrite(dac_sync_pins[channel], LOW);

        for (uint8_t db = 0; db < msg2.block_size; db++) {
            data[db] = SPI.transfer(msg2.msg[block * msg2.block_size + db]);       
        }
        digitalWrite(dac_sync_pins[channel], HIGH);

    }

    double voltage = threeByteToVoltage(data[0], data[1], data[2]);
    return(voltage);

}

uint32_t AD5791::threeByteToInt(uint8_t DB1, uint8_t DB2, uint8_t DB3) {
    return ((uint32_t)(((((DB1 & 15) << 8) | DB2) << 8) | DB3));
}

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


