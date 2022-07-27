#include "include/ad5791.h"
#include "include/utils.h"
//#include <stdint.h>
#include <SPI.h>

uint8_t channels[4] = {6, 6, 6, 6}; //Define!
AD5791 dac(channels, 4);


void setup() {
  Serial.begin(115200);
  dac.Begin(); //DONE
  dac.Initialize();
}

uint8_t Router(String cmd[], uint8_t cmd_size) {

  String command = cmd[0];
  double voltage;
  
  if (command == "NOP") { //DONE
    Serial.println("NOP");
    return 0;
  }
  else if (command == "*IDN?") { //DONE
    Serial.println(dac.name);
    return 0;
  }
  else if (command == "*RDY?") { //DONE
    Serial.println("READY");
    return 0;
  }
  else if (command == "DACWRITE") { //DONE! -- need test
    voltage = dac.SetVoltage(cmd[1].toInt(), cmd[2].toFloat(), true);
    
    Serial.print("DAC #");
    Serial.println(cmd[1].toInt());
    Serial.print("UPDATED TO ");
    Serial.print(voltage, 5);
    Serial.println("V");
  }
  else if (command == "GETDAC") { //DONE! -- need test
    voltage = dac.readDAC(cmd[1].toInt());
    Serial.print("DAC #");
    Serial.println(cmd[1].toInt());
    Serial.print("LAST UPDATED TO ");
    Serial.print(voltage, 5);
    Serial.println("V");
  }
  // else if (command == "GETADC") {
  //   voltage = readADC(DB[1].toInt());
  //   Serial.println(voltage,7);
  // }
}


void loop() {

  Serial.flush();
  
  if (Serial.available()) {
      
      String cmd[30];
      uint8_t cmd_size;
      
      cmd_size = interface_utils::query_serial(cmd);
      
      for (int i = 0; i < cmd_size; i++) {
          Serial.println(cmd[i]);
      }
      Router(cmd, cmd_size);
   }
}
