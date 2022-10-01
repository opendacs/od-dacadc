#include "include/ad5791.h"
#include "include/ad4115.h"
//#include "include/ramp.h"
#include "include/utils.h"
#include <SPI.h>
#include <stdint.h>

using namespace std;

uint8_t channels[4] = {6, 6, 6, 6}; //DACs pins
AD5791 dac(channels, 4);

AD4115 adc(32, 28);

void setup() {
  Serial.begin(115200);
  dac.Begin(); 
  dac.Initialize();
  adc.reset_adc();
}

uint8_t Router(String cmd[], uint8_t cmd_size) {
  
  String command = cmd[0];
  double voltage;
  

  //DAC COMMANDS SECTION
  if (command == "DACWRITE") {
    voltage = dac.SetVoltage(cmd[1].toInt(), cmd[2].toFloat(), true);
    
    Serial.print("DAC #");
    Serial.println(cmd[1].toInt());
    Serial.print("UPDATED TO ");
    Serial.print(voltage, 5);
    Serial.println("V");
  }

  else if (command == "GET_DAC") {
    voltage = dac.readDAC(cmd[1].toInt());
    Serial.print("DAC #");
    Serial.println(cmd[1].toInt());
    Serial.print("LAST UPDATED TO ");
    Serial.print(voltage, 5);
    Serial.println("V");
  }

  //ADC COMMANDS SECTION
  else if (command == "GET_ADC") {
    voltage = adc.full_reading();
    // Serial.print("READING ");
    // Serial.print(voltage,7);
    // Serial.print("V");
  }

  else if (command == "CONFIG_CHANNEL") {
    voltage = adc.config_channel(cmd[1].toInt(), cmd[2].toInt(), cmd[3].toInt(), cmd[4].toInt(), cmd[5].toInt());
  }

  else if (command == "ADC_CONFIG") {
    uint8_t data = adc.general_config(cmd[1].toInt(), cmd[2].toInt(), cmd[3].toInt(), cmd[4].toInt(), cmd[5].toInt());
    Serial.println(data);
  }

  else if (command == "SETUP_CONFIG") {
    voltage = adc.setup_config();
  }

  else if (command == "DISABLE_ALL_CHANNELS") {
    uint8_t data = adc.disable_all_channels();
    Serial.println(data);
  }

  //RAMP FUNCTIONS SECTION
  // else if (command == "BUFFER_RAMP") {
  //   for (int i = 0; i < cmd_size; i++){
  //     ramp_param[i] = cmd[i].toInt();  
  //   }
  //   ramp.buffer();
  // }

  //DEBUGGING COMMANDS SECTION
  else if (command == "NOP") {
    Serial.println("NOP");
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
    uint8_t id = adc.read_id();
    Serial.print("ID code is ");
    Serial.println(id);
  }
}

void loop() {

  Serial.flush();
  
  if (Serial.available()) {
      
      String cmd[30];
      uint8_t cmd_size;
      
      cmd_size = interface_utils::query_serial(cmd);
      
      Router(cmd, cmd_size);
   }
}
