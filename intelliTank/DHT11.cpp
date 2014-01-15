
#include "DHT11.h"

// #define TIMEOUT 10000 
// uint16_t for UNO, higher CPU speeds => exceed MAXINT.
// works for DUE
#define TIMEOUT (F_CPU/1600)    


/////////////////////////////////////////////////////
//
// PUBLIC
//

// return values:
// DHTLIB_OK
// DHTLIB_ERROR_CHECKSUM
// DHTLIB_ERROR_TIMEOUT
int dht::read11(uint8_t pin)
{
	// READ VALUES
	int rv = read(pin);
	if (rv != DHTLIB_OK)
    {
		humidity    = DHTLIB_INVALID_VALUE; // invalid value, or is NaN prefered?
		temperature = DHTLIB_INVALID_VALUE; // invalid value
		return rv;
	}

	// CONVERT AND STORE
	humidity    = bits[0];  // bits[1] == 0;
	temperature = bits[2];  // bits[3] == 0;

	// TEST CHECKSUM
	// bits[1] && bits[3] both 0
	uint8_t sum = bits[0] + bits[2];
	if (bits[4] != sum) return DHTLIB_ERROR_CHECKSUM;

	return DHTLIB_OK;
}


/////////////////////////////////////////////////////
//
// PRIVATE
//

// return values:
// DHTLIB_OK
// DHTLIB_ERROR_TIMEOUT
int dht::read(uint8_t pin)
{
	// INIT BUFFERVAR TO RECEIVE DATA
	uint8_t cnt = 7;
	uint8_t idx = 0;

	// EMPTY BUFFER
	for (uint8_t i=0; i< 5; i++) bits[i] = 0;

	// REQUEST SAMPLE
	pinMode(pin, OUTPUT);
	digitalWrite(pin, LOW);
	delay(20);
	digitalWrite(pin, HIGH);
	delayMicroseconds(40);
	pinMode(pin, INPUT);

    // TODO rewrite with miros()?
	// GET ACKNOWLEDGE or TIMEOUT
	unsigned int loopCnt = TIMEOUT;
	while(digitalRead(pin) == LOW)
		if (loopCnt-- == 0) return DHTLIB_ERROR_TIMEOUT;

	loopCnt = TIMEOUT;
	while(digitalRead(pin) == HIGH)
		if (loopCnt-- == 0) return DHTLIB_ERROR_TIMEOUT;

	// READ THE OUTPUT - 40 BITS => 5 BYTES
	for (uint8_t i=0; i<40; i++)
	{
		loopCnt = TIMEOUT;
		while(digitalRead(pin) == LOW)
			if (loopCnt-- == 0) return DHTLIB_ERROR_TIMEOUT;

		unsigned long t = micros();

		loopCnt = TIMEOUT;
		while(digitalRead(pin) == HIGH)
			if (loopCnt-- == 0) return DHTLIB_ERROR_TIMEOUT;

		if ((micros() - t) > 40) bits[idx] |= (1 << cnt);
		if (cnt == 0)   // next byte?
		{
			cnt = 7;   
			idx++;      
		}
		else cnt--;
	}

	return DHTLIB_OK;
}
