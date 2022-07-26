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
      double voltage;
      voltage = dac.SetVoltage(cmd[1].toInt(), cmd[2].toFloat(), true);
      if (voltage == 999) {
          Serial.println("VOLTAGE OVERRANGE");
      }
      else {
          Serial.print("VOLTAGE UPDATED TO ");
          Serial.println(voltage);
      }
    }
    else if (command == "GETDAC") { //DONE! -- need test
        double data = dac.readDAC(cmd[1].toInt());
        Serial.println("Voltage is ");
        Serial.println(data);
    }

}


void loop() {

    Serial.flush();
    if (Serial.available()) {
        String cmd[30];
        uint8_t cmd_size;
        cmd_size = interface_utils::query_serial(cmd);
        for(int i = 0; i < cmd_size; i++) {
            Serial.println(cmd[i]);
        }
        Router(cmd, cmd_size);
   }
}