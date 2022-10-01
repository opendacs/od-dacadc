// #include "../include/ramp.h"
// #include <stdint.h>
// #include <SPI.h>
// #include <cstdlib>
// #include <WString.h>
// #include <Arduino.h>
// using namespace std;

// uint8_t RAMP::BUFFER(void) {

//   String channelsDAC = ramp_cmd[1];
//   int NchannelsDAC = channelsDAC.length();
//   String channelsADC = ramp_cmd[2];
//   int NchannelsADC = channelsADC.length();
  
//   uint8_t vi[16];
//   uint8_t vf[16];
  
//   float v_min = -1*DAC_FULL_SCALE;
//   float v_max = DAC_FULL_SCALE;
  
//   for (int i = 3; i < NchannelsDAC + 3; i++) {
// 	vi[i-3] = ramp_cmd[i].toFloat();
// 	vf[i-3] = ramp_cmd[i+NchannelsDAC].toFloat();  
//   }
  
//   int nSteps = (ramp_cmd[NchannelsDAC*2+3].toInt());
//   byte b1;
//   byte b2;
//   byte b3;
//   int count =0;
  
//   for (int j = 0; j < nSteps; j++) {
//     digitalWrite(data,HIGH);
    
//     for(int i = 0; i < NchannelsDAC; i++)
//     {
//       float v;
//       v = vi[i]+(vf[i]-vi[i])*j/(nSteps-1);
//       if(v<v_min)
//       {
//         v=v_min;
//       }
//       else if(v>v_max)
//       {
//         v=v_max;
//       }
//       writeDAC_buffer(channelsDAC[i]-'0',v);
//     }
//     digitalWrite(ldac,LOW);
//     digitalWrite(ldac,HIGH);
//     if (delayUnit)
//     {
//       delay(DB[NchannelsDAC*2+4].toInt());
//     }
//     else
//     {
//       delayMicroseconds(DB[NchannelsDAC*2+4].toInt());
//     }
//     for(int i = 0; i < NchannelsADC; i++)
//     {
//       rampRead(channelsADC[i]-'0', b1, b2, b3, &b1, &b2, &b3, count,DB[NchannelsDAC*2+5].toInt());
//       count+=1;
//     }
//     if(Serial.available())
//     {
//       std::vector<String> comm;
//       comm = query_serial();
//       if(comm[0] == "STOP")
//       {
//         break;
//       }
//     }
//   }
//   Serial.write(b1);
//   Serial.write(b2);
//   Serial.write(b3);
//   digitalWrite(data,LOW);
// }
