/*
 * HAL.h
 *
 *  Created on: Apr 19, 2019
 *      Author: curiosul
 */

#ifndef _HAL_H_
#define _HAL_H_

#include "Arduino.h"

#define DRIVERS_DEBUG 1

#define ERR_PRINTLN(arg)	(Serial.println(arg))
#define ERR_PRINT(arg)		(Serial.print(arg))
#define DBG_PRINT(arg)		(Serial.print(arg))
#define DBG_PRINTLN(arg)	(Serial.println(arg))

typedef enum
{
    ERR_OK = 0,
    ERR_NOK = 1
}status_t;

#define Vfb_DigitalWrite(pin, level)	digitalWrite(pin, level)
#define Vfb_DigitalRead(pin)			digitalRead(pin)
#define Vfb_DigitalToggle(pin)			digitalWrite(pin, !digitalRead(pin) )
#define Vfb_SetPinMode(pin, mode)		pinMode(pin, mode)
#define Vfb_AnalogRead(pin)				analogRead(pin)

#endif /* _HAL_H_ */
