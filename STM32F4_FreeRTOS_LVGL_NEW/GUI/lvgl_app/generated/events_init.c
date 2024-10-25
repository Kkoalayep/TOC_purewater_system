/*
* Copyright 2024 NXP
* NXP Confidential and Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "events_init.h"
#include <stdio.h>
#include "lvgl.h"
#include "console_logic.h"

#if LV_USE_FREEMASTER
#include "freemaster_client.h"
#endif


static void screen_oxidize_sw_event_handler (lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);

	switch (code) {
	case LV_EVENT_CLICKED:
	{
		lv_obj_t * status_obj = lv_event_get_target(e);
		int status = lv_obj_has_state(status_obj, LV_STATE_CHECKED) ? 1 : 0;
		oxidize_state = (uint8_t) status;
		switch(status) {
		case 0:
		{
			lv_obj_set_style_text_font(guider_ui.screen_oxidize_state_lable, &lv_font_simfang_24, 0);
			lv_label_set_text(guider_ui.screen_oxidize_state_lable, "已停止");
			break;
		}
		case 1:
		{
			lv_obj_set_style_text_font(guider_ui.screen_oxidize_state_lable, &lv_font_simfang_24, 0);
			lv_label_set_text(guider_ui.screen_oxidize_state_lable, "运行中");
			break;
		}
		default:
			break;
		}
		break;
	}
	default:
		break;
	}
}
static void screen_motor_sw_event_handler (lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	
	switch (code) {
	case LV_EVENT_CLICKED:
	{
		lv_obj_t * status_obj = lv_event_get_target(e);
		int status = lv_obj_has_state(status_obj, LV_STATE_CHECKED) ? 1 : 0;
		motor_state = (uint8_t) status;
		switch(status) {
		case 0:
		{
			lv_obj_set_style_text_font(guider_ui.screen_motor_state_lable, &lv_font_simfang_24, 0);
			lv_label_set_text(guider_ui.screen_motor_state_lable, "关");
			break;
		}
		case 1:
		{
			lv_obj_set_style_text_font(guider_ui.screen_motor_state_lable, &lv_font_simfang_24, 0);
			lv_label_set_text(guider_ui.screen_motor_state_lable, "开");
			break;
		}
		default:
			break;
		}
		break;
	}
	default:
		break;
	}
}
static void screen_light_sw_event_handler (lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);

	switch (code) {
	case LV_EVENT_CLICKED:
	{
		lv_obj_t * status_obj = lv_event_get_target(e);
		int status = lv_obj_has_state(status_obj, LV_STATE_CHECKED) ? 1 : 0;
		light_state = (uint8_t) status;
		switch(status) {
		case 0:
		{
			lv_obj_set_style_text_font(guider_ui.screen_light_state_lable, &lv_font_simfang_24, 0);
			lv_label_set_text(guider_ui.screen_light_state_lable, "关");
			break;
		}
		case 1:
		{
			lv_obj_set_style_text_font(guider_ui.screen_light_state_lable, &lv_font_simfang_24, 0);
			lv_label_set_text(guider_ui.screen_light_state_lable, "开");
			break;
		}
		default:
			break;
		}
		break;
	}
	default:
		break;
	}
}
static void screen_svo_sw_event_handler (lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);

	switch (code) {
	case LV_EVENT_CLICKED:
	{
		lv_obj_t * status_obj = lv_event_get_target(e);
		int status = lv_obj_has_state(status_obj, LV_STATE_CHECKED) ? 1 : 0;
		svo_state = (uint8_t) status;
		switch(status) {
		case 0:
		{
			lv_obj_set_style_text_font(guider_ui.screen_svo_state_lable, &lv_font_simfang_24, 0);
			lv_label_set_text(guider_ui.screen_svo_state_lable, "关");
			break;
		}
		case 1:
		{
			lv_obj_set_style_text_font(guider_ui.screen_svo_state_lable, &lv_font_simfang_24, 0);
			lv_label_set_text(guider_ui.screen_svo_state_lable, "开");
			break;
		}
		default:
			break;
		}
		break;
	}
	default:
		break;
	}
}
static void screen_clear_sw_event_handler (lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);

	switch (code) {
	case LV_EVENT_CLICKED:
	{
		lv_obj_t * status_obj = lv_event_get_target(e);
		int status = lv_obj_has_state(status_obj, LV_STATE_CHECKED) ? 1 : 0;
		clear_state = (uint8_t) status;
		switch(status) {
		case 0:
		{
			lv_obj_set_style_text_font(guider_ui.screen_clear_state_lable, &lv_font_simfang_24, 0);
			lv_label_set_text(guider_ui.screen_clear_state_lable, "已停止");
			break;
		}
		case 1:
		{
			lv_obj_set_style_text_font(guider_ui.screen_clear_state_lable, &lv_font_simfang_24, 0);
			lv_label_set_text(guider_ui.screen_clear_state_lable, "运行中");
			break;
		}
		default:
			break;
		}
		break;
	}
	default:
		break;
	}
}
static void screen_svt_sw_event_handler (lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);

	switch (code) {
	case LV_EVENT_CLICKED:
	{
		lv_obj_t * status_obj = lv_event_get_target(e);
		int status = lv_obj_has_state(status_obj, LV_STATE_CHECKED) ? 1 : 0;
		svt_state = (uint8_t) status;
		switch(status) {
		case 0:
		{
			lv_obj_set_style_text_font(guider_ui.screen_svt_state_lable, &lv_font_simfang_24, 0);
			lv_label_set_text(guider_ui.screen_svt_state_lable, "关");
			break;
		}
		case 1:
		{
			lv_obj_set_style_text_font(guider_ui.screen_svt_state_lable, &lv_font_simfang_24, 0);
			lv_label_set_text(guider_ui.screen_svt_state_lable, "开");
			break;
		}
		default:
			break;
		}
		break;
	}
	default:
		break;
	}
}
void events_init_screen(lv_ui *ui)
{
	lv_obj_add_event_cb(ui->screen_oxidize_sw, screen_oxidize_sw_event_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(ui->screen_motor_sw, screen_motor_sw_event_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(ui->screen_light_sw, screen_light_sw_event_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(ui->screen_svo_sw, screen_svo_sw_event_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(ui->screen_clear_sw, screen_clear_sw_event_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(ui->screen_svt_sw, screen_svt_sw_event_handler, LV_EVENT_ALL, NULL);
}

void events_init(lv_ui *ui)
{

}
