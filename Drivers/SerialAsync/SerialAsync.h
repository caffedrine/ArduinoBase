#ifndef SERIAL_ASYNC_H
#define SERIAL_ASYNC_H

#include <Arduino.h>

#ifndef SERIAL_ASYNC_STATIC_BUFFER_SIZE
	#define SERIAL_ASYNC_STATIC_BUFFER_SIZE	1024u
#endif

#ifndef SERIAL_ASYNC_DEBUG
	#define SERIAL_ASYNC_DEBUG	0
#endif

namespace Drivers
{
	enum class SerialSpeed : uint32_t
	{
		// Very slow speeds (legacy/debugging)
		BAUD_110    = 110,      // Teletype machines
		BAUD_300    = 300,      // Very old modems
		BAUD_600    = 600,      // Old acoustic couplers
		BAUD_1200   = 1200,     // Early modems
		BAUD_2400   = 2400,     // Vintage modems
		BAUD_4800   = 4800,     // Legacy systems

		// Standard low speeds
		BAUD_9600   = 9600,     // Most common default, Arduino default
		BAUD_14400  = 14400,    // Some modems
		BAUD_19200  = 19200,    // Common for embedded systems
		BAUD_28800  = 28800,    // Some applications
		BAUD_38400  = 38400,    // Common for GPS modules

		// Standard medium speeds
		BAUD_57600  = 57600,    // Common for development
		BAUD_76800  = 76800,    // Less common
		BAUD_115200 = 115200,   // Very common for development/debugging

		// High speeds
		BAUD_230400 = 230400,   // High-speed applications
		BAUD_250000 = 250000,   // 3D printers (Marlin firmware)
		BAUD_460800 = 460800,   // Very high speed
		BAUD_500000 = 500000,   // Some applications
		BAUD_576000 = 576000,   // Less common
		BAUD_921600 = 921600    // Maximum for many systems
	};

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
