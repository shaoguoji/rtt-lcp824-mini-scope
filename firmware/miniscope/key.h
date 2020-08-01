#ifndef __KEY_H__
#define __KEY_H__

#include <rtthread.h>

#include <stdio.h>
#include "board.h"

#define LPC824_READ_PIN(pin)	Chip_GPIO_PinGetState(LPC_GPIO_PORT,0,pin)

/* key pin number */
#define PIN_KEY_LEFT	10
#define PIN_KEY_RIGHT	16
#define PIN_KEY_UP		26
#define PIN_KEY_DOWN	27
#define PIN_KEY_OK		11

/* key check time(unit:10ms) */
#define KEY_SHAKE_TIME			3
#define KEY_hold_time_MAX		50
// #define KEY_LONG_hold_time_MIN	100

/* event flag */
#define EVENT_KEY_LEFT_PRESS (1 << PIN_KEY_LEFT)
#define EVENT_KEY_RIGHT_PRESS (1 << PIN_KEY_RIGHT)
#define EVENT_KEY_UP_PRESS (1 << PIN_KEY_UP)
#define EVENT_KEY_DOWN_PRESS (1 << PIN_KEY_DOWN)
#define EVENT_KEY_OK_PRESS (1 << PIN_KEY_OK)

#endif /* #ifndef __KEY_H__ */
