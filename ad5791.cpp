#include "ad5791.h"
#include "utils.h"


dac_utils::Message AD5791::Initialize_Msg(void) {

    dac_utils::Message msg;
    msg.block_size = 3;
    msg.n_blocks = 1;
    msg.msg[0] = 0x20;
    msg.msg[1] = 0x00;
    msg.msg[2] = 0x02;
    return msg;
}

dac_utils::Message AD5791::ReadDac_Msg(void) {

    dac_utils::Message msg;
    msg.block_size = 3;
    msg.n_blocks = 1;
    msg.msg[0] = 0x90; //Command byte
    msg.msg[1] = 0x00;
    msg.msg[2] = 0x00;
    return msg;
}

dac_utils::Message AD5791::threeNullBytes_Msg(void) {

    dac_utils::Message msg;
    msg.block_size = 3;
    msg.n_blocks = 1;
    msg.msg[0] = 0x00; //Command byte
    msg.msg[1] = 0x00;
    msg.msg[2] = 0x00;
    return msg;
}

dac_utils::Message AD5791::SetVoltage_Msg(uint8_t channel, double voltage) {

    uint32_t decimal;
    dac_utils::Message msg;
    msg.block_size = 3;
    msg.n_blocks = 1;

    // The conversion below is for two's complement
    if (voltage < 0) {
        decimal = voltage*524288/dac_utils::DAC_FULL_SCALE + 1048576;
    }
    else {
        decimal = voltage*524287/dac_utils::DAC_FULL_SCALE;
    }

    // Check datasheet for details
    msg.msg[0] = (byte)((decimal >> 0x10) | 0x10);  // Writes to dac register
    msg.msg[1] = ((byte)((decimal >> 0x8) & 0xFF));  // Writes first byte
    msg.msg[2] = ((byte)(decimal & 0xFF));  // Writes second byte
    return msg;
}

double AD5791::BytesToVoltage(dac_utils::Message message) {

    byte byte1 = message.msg[0];
    byte byte2 = message.msg[1];
    byte byte3 = message.msg[2];

    // The conversion below is for two's complement
    uint32_t decimal = ((uint32_t)(((((byte1 & 0x15) << 0x8) | byte2) << 0x8) | byte3));
    double voltage;
    if (decimal > 524287) {
        voltage = -(1048576-decimal)*dac_utils::DAC_FULL_SCALE/524288;
    }
    else {
        voltage = decimal*dac_utils::DAC_FULL_SCALE/524287;
    }
    return voltage;
}

void AD5791::UpdateAnalogOutputs(void) {
    digitalWrite(ldac_pin, LOW);
    digitalWrite(ldac_pin, HIGH);
}

uint8_t AD5791::intToThreeBytes(int decimal, byte *DB1, byte *DB2, byte *DB3) {

    *DB1 = (byte) ((decimal >> 16) | 16);
    *DB2 = (byte) ((decimal >> 8) & 255);
    *DB3 = (byte) (decimal & 255);
}

AD5791::AD5791(uint8_t sync_pins[n_channels], uint8_t ldac_pin, uint8_t  spi_mode, BitOrder bit_order) {

    dac_utils::LDAC = ldac_pin;

    for (int i = 0; i < n_channels; ++i) {
        dac_sync_pins[i] = sync_pins[i];
    }

    while (spi_mode) {

        if (spi_mode == 0) {
            if (bit_order == "MSBFIRST") {
                SPISettings dacSettings(1000000, MSBFIRST, SPI_MODE0);
            }
            else if (bit_order == "LSBFIRST") {
                SPISettings dacSettings(1000000, LSBFIRST, SPI_MODE0);
            }
            else {
                Serial.println("INVALID BIT ORDER");
            }
        }

        if (spi_mode == 1) {
            if (bit_order == "MSBFIRST") {
                SPISettings dacSettings(1000000, MSBFIRST, SPI_MODE1);
            }
            else if (bit_order == "LSBFIRST") {
                SPISettings dacSettings(1000000, LSBFIRST, SPI_MODE1);
            }
            else {
                Serial.println("INVALID BIT ORDER");
            }
        }

        if (spi_mode == 2) {
            if (bit_order == "MSBFIRST") {
                SPISettings dacSettings(1000000, MSBFIRST, SPI_MODE2);
            }
            else if (bit_order == "LSBFIRST") {
                SPISettings dacSettings(1000000, LSBFIRST, SPI_MODE2);
            }
            else {
                Serial.println("INVALID BIT ORDER");
            }
        }

        if (spi_mode == 3) {
            if (bit_order == "MSBFIRST") {
                SPISettings dacSettings(1000000, MSBFIRST, SPI_MODE3);
            }
            else if (bit_order == "LSBFIRST") {
                SPISettings dacSettings(1000000, LSBFIRST, SPI_MODE3);
            }
            else {
                Serial.println("INVALID BIT ORDER");
            }
        }
        else {
            Serial.println("INVALID SPI MODE");
        }
    }

}

uint8_t AD5791::Initialize(void) {

    SPI.beginTransaction(dacSettings);

    dac_utils::Message msg;
    msg = Initialize_Msg();

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
    pinMode(ldac_pin, OUTPUT);

    // Setting LDAC value
    digitalWrite(ldac_pin, HIGH);

    // Initializing and configuring SPI
    SPI.begin();
}

double AD5791::SetVoltage(uint8_t channel, double voltage, bool update_outputs) {

    dac_utils::Message msg;
    msg = SetVoltage_Msg(channel, voltage);

    SPI.beginTransaction(dacSettings);

    if (voltage < -1*dac_utils::DAC_FULL_SCALE || voltage > dac_utils::DAC_FULL_SCALE) {
        return 999;
    }

    else {

        for (uint8_t block = 0; block < msg.n_blocks; block++) {

        digitalWrite(channel, LOW);

        for (uint8_t db = 0; db < msg.block_size; db++) {

            SPI.transfer(msg.msg[block * msg.block_size + db]);
        }

        digitalWrite(channel, HIGH);
        }

      if (update_outputs) {

        UpdateAnalogOutputs();  // Analog output updated

      }

      // Updated voltage may be different than voltage parameter because of
      // resolution
      return BytesToVoltage(msg);
    }
}

uint8_t AD5791::readDAC(uint8_t channel) {
    dac_utils::Message msg;
    msg = ReadDac_Msg();

    for (uint8_t block = 0; block < msg.n_blocks; block++) {

        digitalWrite(channel, LOW);

        for (uint8_t db = 0; db < msg.block_size; db++) {

            SPI.transfer(msg.msg[block * msg.block_size + db]);
        }

        digitalWrite(channel, HIGH);

    }

    delayMicroseconds(1);

    uint8_t data[3];
    dac_utils::Message msg2;
    msg = threeNullBytes_Msg();

    for (uint8_t block = 0; block < msg2.n_blocks; block++) {

        digitalWrite(channel, LOW);

        for (uint8_t db = 0; db < msg2.block_size; db++) {

            data[db] = SPI.transfer(msg2.msg[block * msg2.block_size + db]);
        }

        digitalWrite(channel, HIGH);

    }

    uint8_t voltage = threeByteToVoltage(data[0], data[1], data[2]);
    Serial.println(voltage,5);

}

uint8_t AD5791::threeByteToInt(uint8_t DB1, uint8_t DB2, uint8_t DB3) {
    return ((int)(((((DB1&15)<<8)| DB2)<<8)|DB3));
}

uint8_t AD5791::threeByteToVoltage(uint8_t DB1, uint8_t DB2, uint8_t DB3) {

    int decimal;
    uint8_t voltage;

    decimal = threeByteToInt(DB1,DB2,DB3);

    if (decimal <= 524287) {
        voltage = decimal*dac_utils::DAC_FULL_SCALE/524287;
    }
    else {
        voltage = -(1048576-decimal)*dac_utils::DAC_FULL_SCALE/524288;
      }
    return voltage;
}



