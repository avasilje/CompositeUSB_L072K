#pragma once

typedef struct imon_s {

	const int16_t *adc_ts;
	const int16_t *adc_vrefint;

	int adc_ready;

	uint32_t ts_slope;
	uint32_t ts_offset;

	int16_t temp_degc;
	uint16_t vref;

	int last_report_tick;

} imon_t;

extern imon_t g_imon;

void imon_adc_completed();
void imon_on_idle(uint32_t now_tick);
void imon_init(int16_t *adc_ts, int16_t *adc_vrefint);
