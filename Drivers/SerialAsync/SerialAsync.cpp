/*
 * SerialAsync.cpp
 *
 *  Created on: Apr 19, 2019
 *      Author: curiosul
 */

#include "SerialAsync.h"

#ifndef SERIAL_ASYNC_DEBUG
	#define SERIAL_ASYNC_DEBUG	0
#endif

namespace Drivers
{
	SerialAsync::SerialAsync(HardwareSerial *serialChannel, uint32_t BaudRate)
	{
		this->serial = serialChannel;
		this->serial->begin(BaudRate);
	}

	uint16_t SerialAsync::WriteString(const String &str)
	{
		return this->WriteBytes((uint8_t *)str.c_str(), (uint16_t)str.length());
	}

	uint16_t SerialAsync::WriteBytes(uint8_t *bytes, uint16_t bytes_length)
	{
		// If size if too big then return 0 as not of the bytes will be written
		uint16_t InternalBufferAvailability, SerialBufferAvailability;

        InternalBufferAvailability = this->BufferAvailability();
        SerialBufferAvailability = this->serial->availableForWrite();

		if( bytes_length > (InternalBufferAvailability + SerialBufferAvailability) )
			return 0;

		#if SERIAL_ASYNC_DEBUG
		printf("\nWrite %d bytes\n", bytes_length);
		printf("  -> Internal buffer availability: %d bytes\n", InternalBufferAvailability);
		printf("  -> Serial buffer availability  : %d bytes\n", SerialBufferAvailability);
		printf("  -> Total capacity : %d bytes\n", InternalBufferAvailability + SerialBufferAvailability);
		#endif
		// If size fits in available serial buffer and internal buffer is empty then just send it serial buffer
		if( (InternalBufferAvailability == SERIAL_ASYNC_STATIC_BUFFER_SIZE) && (SerialBufferAvailability >= bytes_length))
		{
			return this->serial->write(bytes, bytes_length);
		}
		else // received data needs to be splitted and send a part to serial and a part is enqueued
		{
			// Send to serial next bytes to be written from buffer
			if( SerialBufferAvailability > 0 )
			{
				// Shift to serial the bytes available internally
				if( InternalBufferAvailability < SERIAL_ASYNC_STATIC_BUFFER_SIZE)
				{
					// At this point, there is available space inside serial buffer
					// and there are as well bytes into internal serial
					// Shift out bytes from internal buffer to serial and place remaining bytes into
					// correct buffer or both if the case

					// Write data from internal buffer to serial
					uint16_t w_len = (((SERIAL_ASYNC_STATIC_BUFFER_SIZE - InternalBufferAvailability) > SerialBufferAvailability) ? (SerialBufferAvailability) : (SERIAL_ASYNC_STATIC_BUFFER_SIZE - InternalBufferAvailability));
					// Loop through all bytes that needs to be send
					for( uint16_t i = 0; i < w_len; i++ )
					{
						this->serial->write(this->BufferPop());
					}
					#if SERIAL_ASYNC_DEBUG
					printf("Transfer %d bytes from internal buffer to serial buffer\n", w_len);
					#endif

					// Read again the lengths of the buffers
			        InternalBufferAvailability = this->BufferAvailability();
			        SerialBufferAvailability = this->serial->availableForWrite();

			        // If there is still available space in serial buffer shift out some of the bytes from request
			        if( SerialBufferAvailability > 0 )
			        {
						// Internal buffer is empty. Write as much possible into serial and push the rest into internal buffer
						uint16_t w_len = ((bytes_length > SerialBufferAvailability) ? (SerialBufferAvailability) : (bytes_length));
						this->serial->write(bytes, w_len);

						// push remaining bytes to internal buffer
						uint16_t remaining_bytes = bytes_length - w_len;
						this->BufferPush(&bytes[w_len], remaining_bytes);

						#if SERIAL_ASYNC_DEBUG
						printf("Pushed %d bytes into serial buffer and %d bytes into internal buffer\n", w_len, remaining_bytes);
						printf("Internal buffer queued bytes %d\n", SERIAL_ASYNC_STATIC_BUFFER_SIZE - this->BufferAvailability());
						#endif
			        }
					else // Serial buffer is full. Append remaining bytes to internal buffer
					{
						this->BufferPush(bytes, bytes_length);
						#if SERIAL_ASYNC_DEBUG
						printf("Pushed %d bytes into internal buffer\n", bytes_length);
						#endif
					}
				}
				else
				{
					// Internal buffer is empty. Write as much possible into serial and push the rest into internal buffer
					uint16_t w_len = ((bytes_length > SerialBufferAvailability) ? (SerialBufferAvailability) : (bytes_length));
					this->serial->write(bytes, w_len);

					// push remaining bytes to internal buffer
					uint16_t remaining_bytes = bytes_length - w_len;
					this->BufferPush(&bytes[w_len], remaining_bytes);
					#if SERIAL_ASYNC_DEBUG
					printf("Pushed %d bytes into serial buffer and %d bytes into internal buffer\n", w_len, remaining_bytes);
					printf("Internal buffer queued bytes %d\n", SERIAL_ASYNC_STATIC_BUFFER_SIZE - this->BufferAvailability());
					#endif
				}
			}
			else // No space in serial buffer. Write everything in internal static buffer
			{
				this->BufferPush(bytes, bytes_length);
				#if SERIAL_ASYNC_DEBUG
				printf("Pushed %d bytes into internal buffer\n", bytes_length);
				#endif
			}
		}

		return bytes_length;
	}

	void SerialAsync::MainFunction()
	{
		// Check whether there are bytes queued to be send
		if( this->BufferAvailability() < SERIAL_ASYNC_STATIC_BUFFER_SIZE)
		{
			uint16_t InternalBufferAvailability, SerialBufferAvailability;
	        InternalBufferAvailability = this->BufferAvailability();
	        SerialBufferAvailability = this->serial->availableForWrite();

			uint16_t write_len = (((SERIAL_ASYNC_STATIC_BUFFER_SIZE - InternalBufferAvailability) > SerialBufferAvailability) ? (SerialBufferAvailability) : (SERIAL_ASYNC_STATIC_BUFFER_SIZE - InternalBufferAvailability));
			// Loop through all bytes that needs to be send
			for( uint16_t i = 0; i < write_len; i++ )
			{
				this->serial->write(this->BufferPop());
			}
			#if SERIAL_ASYNC_DEBUG
			printf("Mainfunction write %d bytes\n", write_len);
			#endif
		}
	}

} /* namespace Drivers */
