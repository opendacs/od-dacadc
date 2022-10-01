#ifndef BUFFER_H
#define BUFFER_H
#include <SPI.h>
#include <stdint.h>
#include "utils.h"
using namespace std;

class RAMP
{
protected:


private:



public:
	uint8_t ramp_cmd[20];
	uint8_t buffer(void);

};


#endif // BUFFER_H