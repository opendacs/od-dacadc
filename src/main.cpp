#include "ad5791.h"
#include "utils.h"

uint8_t channels[4]; //Define!
AD5791 dac(channels, 10, 1, MSBFIRST);



void setup() {
  Serial.begin(115200);
  dac.Begin(); //DONE
  dac.Initialize();//DONE
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
          Serial.println(voltage);
      }
    }
    else if (command == "GET_DAC") { //DONE! -- need test
        uint8_t data = dac.readDAC(cmd[1].toInt());
        Serial.println(data);
    }
}


void loop() {

    Serial.flush();
    if (Serial.available()) {
        String cmd[30];
        uint8_t cmd_size = interface_utils::query_serial(cmd);
        for(int i=0;i<cmd_size;i++) {
            Serial.println(cmd[i]);
        }
        Router(cmd, cmd_size);
   }
}
