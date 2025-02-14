#ifndef __MY_CONFIG_H
#define __MY_CONFIG_H

#include "include.h"

#define USE_MY_DEBUG 0

#if USE_MY_DEBUG
#include <stdio.h>
#endif

#include "adc.h"
#include "tmr1.h"
#include "tmr2.h"

#include "motor.h"
#include "key.h"
#include "led.h"

#include "heating.h"
#include "charge.h"

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

#endif

