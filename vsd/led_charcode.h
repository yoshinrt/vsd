// $Id$

#ifndef _LED_CHARCODE_H
#define _LED_CHARCODE_H

enum {
	#define LED_PATN( val, name ) LED_ ## name,
	#define LED_PAT( val, name )
	#include "led_font.h"
};

enum {
	#define LED_PAT( val, name ) FONT_ ## name = val,
	#include "led_font.h"
};

#endif
