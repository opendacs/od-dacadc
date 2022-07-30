#include "include/ad5791.h"
#include "include/ad4115.h"
#include "include/utils.h"
#include <SPI.h>

uint8_t channels[4] = {6, 6, 6, 6}; //DACs pins
AD5791 dac(channels, 4);

uint8_t adc_sync = 0; //Define!
AD4115 adc(adc_sync);

void setup() {
  Serial.begin(115200);
  dac.Begin(); 
  dac.Initialize();
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

  else if (command == "GETDAC") {
    voltage = dac.readDAC(cmd[1].toInt());
    Serial.print("DAC #");
    Serial.println(cmd[1].toInt());
    Serial.print("LAST UPDATED TO ");
    Serial.print(voltage, 5);
    Serial.println("V");
  }

  //ADC COMMANDS SECTION
  else if (command == "GET_ADC") {
    voltage = adc.single_reading(cmd[1].toInt());
    Serial.print("READING ");
    Serial.print(voltage,7);
    Serial.print("V");
  }

  else if (command == "ADC_CONFIG") {
    uint8_t data = adc.general_config(cmd[1], cmd[2], cmd[3], cmd[4], cmd[5]);
    Serial.println(data);
  }

  else if (command == "SETUP_CONFIG") {
    Serial.println("FEATURE UNDER DEVELOPMENT");
  }

  else if (command == "DISABLE_ALL_CHANNELS") {
    uint8_t data = adc.disable_all_channels();
    Serial.println(data);
  }

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
