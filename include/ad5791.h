#ifndef AD5791_H
#define AD5791_H
#include <SPI.h>
#include <stdint.h>
#include "utils.h"
using namespace std;


typedef unsigned char byte;

class AD5791 
{
protected:

    virtual spi_utils::Message Initialize_Msg(void);
    virtual spi_utils::Message SetVoltage_Msg(double voltage);
    virtual spi_utils::Message ReadDac_Msg(void);
    virtual spi_utils::Message threeNullBytes_Msg(void);
    virtual double BytesToVoltage(spi_utils::Message message);

private:

    static const uint8_t n_channels = 4;
    uint8_t ldac_pin, spi_mode;
    
    // dac_sync_pins identifies the dac chip; each chip has a unique sync_pin.
    uint8_t dac_sync_pins[n_channels];

    //Numbering system conversions
    uint8_t intToThreeBytes(int decimal, byte *DB1, byte *DB2, byte *DB3);
    uint32_t threeByteToInt(byte DB1,byte DB2, byte DB3);
    double threeByteToVoltage(uint8_t DB1, uint8_t DB2, uint8_t DB3);
    uint8_t voltageToDecimal(float voltage, byte *DB1, byte *DB2, byte *DB3);

public:
    AD5791(void) = default;
    SPISettings dacSettings = SPISettings(1000000, MSBFIRST, SPI_MODE1);
    String name = "DACNAMEHERE";
    float const DAC_FULL_SCALE = 10.0;
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
    int dac[n_channels] = {12, 13, 14, 15}; //Define!
    float GE[n_channels] = {1, 1, 1, 1}; // Offset error
    float OS[n_channels] = {0, 0, 0, 0}; // Gain error
    double SetVoltage(uint8_t channel, double voltage, bool update_outputs);
    double readDAC(uint8_t channel);
    ///
    ///
    ///
    /// Constructor
    /// \param[in] sync_pin The sync or chip select of the dac chip. Different than spi_bus_config_pin
    /// \param[in] ldac_pin The ldac pin used to update the analog outputs.
    ///
    AD5791(uint8_t sync_pins[n_channels], uint8_t ldac_pin);
    ///
    ///
    /// Updates the outputs of all Dac objects sharing the same ldac_pin_.
    ///
    void UpdateAnalogOutputs(void);
};
#endif // AD5791_H

