#ifndef __CONSOLE_LOGIC_H__
#define __CONSOLE_LOGIC_H__

#include "lvgl.h"
#include "events_init.h"
#include "light.h"
#include "motor.h"
#include "svo.h"
#include "svt.h"
#include "led.h"
#include "toc_sensor.h"
#include "stdio.h"
#include "string.h"


void console_logic_run(uint8_t motor_s,uint8_t light_s,uint8_t svo_s,uint8_t svt_s,uint8_t oxidize_s,uint8_t clear_s);

extern uint8_t  oxidize_state;
extern uint8_t  clear_state;


#endif



