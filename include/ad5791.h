#ifndef AD5791_H
#define AD5791_H
#include <SPI.h>
#include <stdint.h>
#include "utils.h"
//#include "ramp.h"
using namespace std;


typedef unsigned char byte;

class AD5791 
{
protected:

    virtual spi_utils::Message initializeMsg(void);
    virtual spi_utils::Message setVoltageMsg(double voltage);
    virtual spi_utils::Message readDacMsg(void);
    virtual spi_utils::Message threeNullBytesMsg(void);
    virtual double bytesToVoltage(spi_utils::Message message);

private:

    static const uint8_t nChannels = 4;
    uint8_t ldacPin, spiMode;
    
    // dacSyncPins identifies the dac chip; each chip has a unique sync_pin.
    uint8_t dacSyncPins[nChannels];

    //Numbering system conversions
    uint8_t intToThreeBytes(int decimal, byte *DB1, byte *DB2, byte *DB3);
    uint32_t threeByteToInt(byte DB1,byte DB2, byte DB3);
    double threeByteToVoltage(uint8_t DB1, uint8_t DB2, uint8_t DB3);
    uint8_t voltageToDecimal(float voltage, byte *DB1, byte *DB2, byte *DB3);

public:
    //AD5791(void) = default;
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
    uint8_t begin(void);
    ///
    ///
    ///
    /// initializes ALL the DACs from tristate mode to normal mode.
    /// \returns 0 if successful.
    ///
    uint8_t initialize(void);
    int LDAC;
    int dac[nChannels] = {12, 13, 14, 15}; //Define!
    float GE[nChannels] = {1, 1, 1, 1}; // Offset error
    float OS[nChannels] = {0, 0, 0, 0}; // Gain error
    double vReadings[4] = {0, 0, 0, 0};
    double setVoltage(uint8_t channel, double voltage, bool updateOutputs);
    double readDac(uint8_t channel);
    double readVoltage(uint8_t channel);
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
    /// \param[in] ldacPin The ldac pin used to update the analog outputs.
    ///
    AD5791(uint8_t syncPins[nChannels], uint8_t ldacPin);
    ///
    ///
    /// Updates the outputs of all Dac objects sharing the same ldacPin_.
    ///

    void updateAnalogOutputs(void);

};
#endif // AD5791_H

