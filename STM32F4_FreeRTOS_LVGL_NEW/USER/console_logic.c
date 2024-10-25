#include "console_logic.h"


uint8_t  oxidize_state = 0;
uint8_t  clear_state = 0;




void console_logic_run(uint8_t motor_s,uint8_t light_s,uint8_t svo_s,uint8_t svt_s,uint8_t oxidize_s,uint8_t clear_s)
{
	
	MOTOR_Console(motor_s);
	LIGHT_Console(light_s);
	SVO_Console(svo_s);
	SVT_Console(svt_s);
	LED_Console(led_state);
}






