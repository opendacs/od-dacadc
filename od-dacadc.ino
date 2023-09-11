/*
  DAC-ADC Data Acquisition System implemented with Arduino Uno

  This program controls the SPI communication between a microcontroller and 
  five peripherals: one AD4115 and four AD5791

  Author: Benjamin Muñoz Cerro - Feldman Lab
  Creation Date: 28 March 2023
*/

#include "include/ad5791.h"
#include "include/ad4115.h"
#include "include/ramp.h"
#include "include/utils.h"
#include <SPI.h>
#include <stdint.h>
#include <cstdlib>

const uint8_t kDacNChannels = 4;
const uint8_t kAdcNChannels = 16;
const uint8_t kAdcSEChannels = 8;
const uint8_t kAdcDFChannels = (kAdcNChannels-kAdcSEChannels)/2;

/**
 * @file main.cpp
 * @brief Initializing objects for DAC, ADC, and RAMPS functionality.
 *
 * This code snippet initializes the necessary objects for DAC, ADC, and RAMPS functionality. It defines an array
 * 'channels' representing the DAC sync pins. The AD5791 object 'dac' is created using the constructor that takes the
 * sync pins array and an 'ldac' pin as parameters. The AD4115 object 'adc' is created using the constructor that takes
 * the sync pin and 'drdy' (MISO) pin as parameters. Finally, the RAMPS object 'ramp_fs' is created using the constructor
 * that takes the 'dac' and 'adc' objects as parameters, allowing the RAMPS object to utilize the functions from the
 * AD5791 and AD4115 classes.
 */
uint8_t channels[4] = {11, 8, 5, 2}; //Dac sync pins

AD5791 dac(channels, 50); //Constructor: sync pins, ldac

AD4115 adc(32, 28); //Constructor: sync pin, drdy(MISO)

RAMPS ramp_fs(dac, adc); //Constructor: ramp_fs uses AD5791 and AD4115 functions.

/**

@brief Setup function for the RAMPS application.
This function initializes the serial communication, DAC, and ADC.
It sets the baud rate of the serial communication to 115200, begins the DAC, and initializes it.
It also resets the ADC.
*/
void setup() {
  Serial.begin(115200);
  dac.begin();

  // Sets DAC outputs to zero before initialization
  for (uint32_t i = 0; i < kDacNChannels; i++) {
    dac.setVoltage(i, 0, true); 
  }
  dac.initialize();
  adc.resetAdc();

  // Dissable all channels
  adc.disableAllChannels();
  
  // Config of all single end channels on Setup0
  for (uint32_t i = 0; i < kAdcSEChannels; i++)
  {
    adc.configChannel(i, 1, 0, i, 16);
  }

  // Config of all differential end channels on Setup0
  uint32_t j = 0;
  for (uint32_t i = kAdcSEChannels; i < kAdcSEChannels+kAdcDFChannels; i++)
  {
    adc.configChannel(i, 1, 0, i+j, i+j+1);
    j++;
  }
  // Only Setup0 supported for now.
  adc.setupConfig();
  adc.interfaceMode();
}

/**
 * @file main.cpp
 * @brief Router function for handling commands and executing corresponding actions.
 *
 * This code snippet defines a function named 'Router' that serves as a router for handling commands and executing
 * corresponding actions. The function takes an array of command strings 'cmd' and the size of the command array 'cmdSize'
 * as parameters. It processes the command and performs the necessary operations based on the command type.
 * 
 * The function first extracts the command and assigns it to the 'command' variable. It also declares a 'voltage' variable
 * for storing voltage values.
 * 
 * The code is divided into sections based on the command type. The DAC COMMANDS SECTION handles commands related to the DAC
 * functionality. It checks the command type and calls the corresponding DAC methods, such as setting the voltage or reading
 * the DAC value. It also prints the results to the Serial monitor.
 * 
 * The ADC COMMANDS SECTION handles commands related to the ADC functionality. It calls the ADC methods to perform operations
 * like reading the ADC value or configuring the channels. It also prints the results to the Serial monitor.
 * 
 * The RAMP FUNCTIONS SECTION handles commands related to the ramp functionality. It extracts the necessary parameters from
 * the command array and calls the appropriate methods from the RAMPS object to perform the ramp operation. It also prints
 * debugging information if uncommented.
 * 
 * The DEBUGGING COMMANDS SECTION handles special debugging commands that perform specific actions, such as printing debug
 * messages or retrieving ID information.
 *
 * Overall, the 'Router' function serves as a central router for interpreting commands and executing the corresponding
 * actions based on the command type. It utilizes the DAC, ADC, and RAMPS objects to perform the required operations.
 */
uint8_t Router(String cmd[], uint8_t cmdSize) {
  
  String command = cmd[0];
  double voltage;
  

  //DAC COMMANDS SECTION
  if (command == "DAC_WRITE") {
    voltage = dac.setVoltage(cmd[1].toInt(), cmd[2].toFloat(), true);
    
    Serial.print("DAC #");
    Serial.print(cmd[1].toInt());
    Serial.print(" | UPDATED TO ");
    Serial.print(voltage, 5);
    Serial.println("V");
  }

  else if (command == "DAC_GET") {
    Serial.println("TESTEST");
    voltage = dac.readDac(cmd[1].toInt());
    Serial.print("DAC #");
    Serial.print(cmd[1].toInt());
    Serial.print(" | LAST UPDATED TO ");
    Serial.print(voltage, 5);
    Serial.println("V");
  }

  else if (command == "INIT_DAC") {
    dac.initialize();
    Serial.println("DAC INITIALIZED");
  }

  //ADC COMMANDS SECTION
  else if (command == "ADC_GET") {
    voltage = adc.fullReading();
    return 0;
  }
  else if (command=="reset_adc"){
    Serial.println("adc reset");
    adc.resetAdc();
  }

  else if (command == "CONFIG_CHANNEL") {
    voltage = adc.configChannel(cmd[1].toInt(), cmd[2].toInt(), cmd[3].toInt(), cmd[4].toInt(), cmd[5].toInt());
  }

  else if (command == "ADC_CONFIG") {
    uint8_t data = adc.generalConfig(cmd[1].toInt(), cmd[2].toInt(), cmd[3].toInt(), cmd[4].toInt(), cmd[5].toInt());
    //Serial.println(data);
  }

  else if (command == "SETUP_CONFIG") {
    voltage = adc.setupConfig();
  }

  else if (command == "DISABLE_ALL_CHANNELS") {
    uint8_t data = adc.disableAllChannels();
    //Serial.println(data);
  }

  //RAMP FUNCTIONS SECTION
  else if (command == "RAMP") {
    //RAMP, 1, 1, 1, 0, 0, 0, 0, 0, 3, 6, 9, 0, 100, 20

    uint8_t channelsDac[4] = {0, 0, 0, 0};
    double vi[4] = {0, 0, 0, 0};
    double vf[4] = {0, 0, 0, 0};

    //Create channelsDAC array of size [4]
    for (int i = 1; i < 5; i++){
      channelsDac[i - 1] = cmd[i].toInt();  
    }

    //Create vi array of size [4]
    for (int i = 5; i < 9; i++){
      vi[i - 5] = std::atof(cmd[i].c_str());  
    }

    //Create vf array of size [4]
    for (int i = 9; i < 13; i++){
      vf[i - 9] = std::atof(cmd[i].c_str());  
    }
    
    //inputs: RAMP, ch1, ch2, ch3, ch4, vi1, vi2, vi3, vi4, vf1, vf2, vf3, vf4, nsteps, delay, buffer
    ramp_fs.simpleRamp(channelsDac, vi, vf, cmd[13].toInt(), std::atof(cmd[14].c_str()), false);
  }

  else if (command == "BUFFER_RAMP") {
    //BUFFER_RAMP, 1, 0, 0, 0, 2, 0, 0, 0, 6, 0, 0, 0, 10, 200

    uint8_t channelsDac[4] = {0, 0, 0, 0};
    double vi[4] = {0, 0, 0, 0};
    double vf[4] = {0, 0, 0, 0};

    //Create channelsDAC array of size [4]
    for (int i = 1; i < 5; i++){
      channelsDac[i - 1] = cmd[i].toInt();  
    }

    //Create vi array of size [4]
    for (int i = 5; i < 9; i++){
      vi[i - 5] = std::atof(cmd[i].c_str());  
    }

    //Create vf array of size [4]
    for (int i = 9; i < 13; i++){
      vf[i - 9] = std::atof(cmd[i].c_str());  
    }

    //inputs: RAMP, ch1, ch2, ch3, ch4, vi1, vi2, vi3, vi4, vf1, vf2, vf3, vf4, nsteps, delay, buffer
    ramp_fs.simpleRamp(channelsDac, vi, vf, cmd[13].toInt(), std::atof(cmd[14].c_str()), true);
  }

  else if (command == "CONFIG_CHANNELS_TEST"){
    adc.configChannelsTest();
    return 0;
  }

  else if (command == "*IDN?") {
    Serial.println(dac.name);
    return 0;
  }

  else if (command == "*RDY?") {
    Serial.println("READY");
    return 0;
  }

  else if (command == "GETID") {
    uint8_t id = adc.readId();
    Serial.print("ID code is ");
    Serial.println(id);
  }

  // if command not found print "NOP\n"
  else {
    Serial.println("NOP");
    return 0;
  }
}

/**
 * @file main.cpp
 * @brief Loop function for processing commands received through the Serial interface.
 *
 * This code snippet defines the main loop function of the program. The loop continuously checks if there is any data available
 * on the Serial interface. If data is available, it reads the incoming command, stores it in the 'cmd' array, and determines
 * the size of the command in the 'cmdSize' variable using the 'interface_utils::querySerial' function.
 *
 * The 'Router' function is then called, passing the 'cmd' array and 'cmdSize' as parameters. The 'Router' function handles the
 * command and performs the corresponding actions based on the command type.
 *
 * The loop function also includes a call to 'Serial.flush()' to ensure that any pending data in the Serial buffer is cleared
 * before processing new commands.
 *
 * Overall, the loop function continuously listens for commands through the Serial interface and processes them using the
 * 'Router' function.
 */
void loop() {

  Serial.flush();
  
  if (Serial.available()) {
      
      String cmd[30];
      uint8_t cmdSize;
      
      cmdSize = interface_utils::querySerial(cmd);
      
      Router(cmd, cmdSize);
   }
}
