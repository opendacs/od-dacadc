#ifndef AD5791_H
#define AD5791_H
#include <stdint.h>
#include <SPI.h>
#include "utils.h"
using namespace std;


typedef unsigned char byte;

namespace dac_utils
{
    static const uint8_t n_channels = 4;
    int const DAC_FULL_SCALE = 10;
    int const DACSCOUNT = 4; //Define!

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


class AD5791
{
protected:

    virtual dac_utils::Message Initialize_Msg(void);
    virtual dac_utils::Message SetVoltage_Msg(double voltage);
    virtual dac_utils::Message ReadDac_Msg(void);
    virtual dac_utils::Message threeNullBytes_Msg(void);
    virtual double BytesToVoltage(dac_utils::Message message);

private:

    static const uint8_t n_channels = 4;
    uint8_t dac_sync_pins[n_channels];
    // sync_pin identifies the dac chip; each chip has a unique sync_pin.
    uint8_t ldac_pin, spi_mode;
    BitOrder bit_order;

public:
    AD5791(void) = default;
    SPISettings dacSettings = SPISettings(1000000, MSBFIRST, SPI_MODE1);
    String name = "DACNAMEHERE";
    ///
    ///
    ///
    /// Configures pins for SPI and initializes SPI communication.
    /// This function must be called before calling any other function.
    /// \returns 0 if success
    ///
    uint8_t Begin(void);
    ///
    ///
    ///
    /// Initializes ALL the DACs from tristate mode to normal mode.
    /// \returns 0 if successful.
    ///
    uint8_t Initialize(void);
    int LDAC;
    int dac[dac_utils::DACSCOUNT] = { 12, 13, 14, 15 }; //Define!
    float GE[dac_utils::DACSCOUNT] = { 1, 1, 1, 1 }; // Offset error
    float OS[dac_utils::DACSCOUNT] = { 0, 0, 0, 0 }; // Gain error
    uint8_t intToThreeBytes(int decimal, byte* DB1, byte* DB2, byte* DB3);
    uint8_t threeByteToInt(byte DB1, byte DB2, byte DB3);
    uint8_t threeByteToVoltage(uint8_t DB1, uint8_t DB2, uint8_t DB3);
    uint8_t voltageToDecimal(float voltage, byte* DB1, byte* DB2, byte* DB3);
    double SetVoltage(uint8_t channel, double voltage, bool update_outputs);
    uint8_t readDAC(uint8_t channel);

    ///
    ///
    ///
    /// Constructor
    /// \param[in] sync_pin The sync or chip select of the dac chip. Different than spi_bus_config_pin
    /// \param[in] spi_bus_config_pin The pin that identifies the bus. More than one dac can share the same pin.
    /// \param[in] ldac_pin The ldac pin used to update the analog outputs.
    /// \param[in] bit_resolution The bit resolution of the dac (eg. 20 for AD5791).
    /// \param[in] clock_divider Sets the spi frequency. spi_freq = clock_freq (84 MHz for arduino due) / clock_divider (default: 4)
    /// \param[in] bit_order Possible values:
    ///   - MSBFIRST Most significant bit first. (default)
    ///   - LSBFIRST Least significant bit first.
    ///
    /// \param[in] spi_mode Possible values:
    ///   - SPI_MODE0
    ///   - SPI_MODE1 (default)
    ///   - SPI_MODE2
    ///   - SPI_MODE3
    ///
    AD5791(uint8_t sync_pins[n_channels], uint8_t ldac_pin);
    ///
    ///
    /// Updates the outputs of all Dac objects sharing the same ldac_pin_.
    ///
    void UpdateAnalogOutputs(void);



};
#endif // AD5791_H

