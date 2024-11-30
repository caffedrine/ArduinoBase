/*
 * SerialAsync.h
 *
 *  Created on: Apr 19, 2019
 *      Author: curiosul
 */

#ifndef SERIAL_ASYNC_H
#define SERIAL_ASYNC_H

#include <Arduino.h>
#include <Printf.h>

#define SERIAL_ASYNC_STATIC_BUFFER_SIZE	1024u

namespace Drivers
{
	class SerialAsync
	{
	public:
		SerialAsync(HardwareSerial *serialChannel, uint32_t BaudRate);
		uint16_t WriteBytes(uint8_t *bytes, uint16_t bytes_length);
		uint16_t WriteString(const String &str);
		void MainFunction();
		inline uint16_t AvailableForWrite()
		{
			return ( ((this->bufferTail - this->bufferHead) + (-((int) (this->bufferTail <= this->bufferHead)) & SERIAL_ASYNC_STATIC_BUFFER_SIZE))) + (((uint16_t)this->serial->availableForWrite()) );
		}

	private:
		uint16_t bufferHead = 0, bufferTail = 0;
		uint8_t buffer[SERIAL_ASYNC_STATIC_BUFFER_SIZE];
		HardwareSerial *serial = nullptr;

		inline uint16_t BufferAvailability()
		{
			return ((this->bufferTail - this->bufferHead) + (-((int) (this->bufferTail <= this->bufferHead)) & SERIAL_ASYNC_STATIC_BUFFER_SIZE));
		}

		inline void BufferPush(uint8_t *bytes, uint16_t bytesLen)
		{
			for( uint16_t i = 0; i < bytesLen; i++ )
			{
				this->buffer[this->bufferHead++] = bytes[i];
				if( this->bufferHead >=  SERIAL_ASYNC_STATIC_BUFFER_SIZE)
				{
					this->bufferHead = 0;
				}
			}
		}

		inline uint8_t BufferPop()
		{
			uint8_t result = this->buffer[this->bufferTail++];
			if( this->bufferTail >= SERIAL_ASYNC_STATIC_BUFFER_SIZE )
			{
				this->bufferTail = 0;
			}

			return result;
		}
	};

} /* namespace Drivers */

#endif /* SERIAL_ASYNC_H */
