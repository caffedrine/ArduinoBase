#include "Arduino.h"
#include <GpioBase.h>
#include <Printf.h>
#include <SerialAsync.h>

// Handler for async serial
Drivers::SerialAsync *uart;


uint8_t buffer[1];
void setup()
{
	pinMode(13, OUTPUT);

	//printf_init(&Serial, 9600);

	// Init asynchronous serial
	uart = new Drivers::SerialAsync(&Serial1, 9600);

	// Init printf for debugging
	printf_init(&Serial, 9600);

	// Init buffer value
	for( uint16_t i = 0; i < sizeof(buffer); i++ )
		buffer[i] = 'a';
}

uint64_t prevMillis = 0, prevMicros = 0;
void loop()
{
	// Trigger async serial main function
	uart->MainFunction();

	// Write serial with 5 characters every 10ms
	if( millis() - prevMillis > 10 )
	{
		prevMillis = millis();
		uart->WriteBytes(buffer, sizeof(buffer));
	}

	// Led used to analyze serial delays on oscilloscope
	uint64_t currMicros = micros();
	if (currMicros - prevMicros >= 100)
	{
		prevMicros = currMicros;
		digitalWrite(13, !digitalRead(13));
	}
}
