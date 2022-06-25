#pragma once

#include "usbd_customhid.h"

#pragma pack(push, 1)
typedef struct {
	uint8_t temperature;
    uint8_t voltage;
} dev0_in_report_t;

typedef struct {
    uint8_t leds;
} dev0_out_report_t;
#pragma pack(pop)

typedef struct {
	uint32_t last_report_tick;
	USBD_HID_Handle *hid;
	dev0_in_report_t hid_in_report;
	dev0_out_report_t hid_out_report;
} dev0_t;

extern dev0_t g_dev0;

void dev0_init();
void dev0_on_idle(uint32_t now_tick);
