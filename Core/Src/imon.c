#include "main.h"
#include "imon.h"

#define QFACT1	10
#define QFACT2  (16-QFACT1)

#define AVG_FACT 3
#define AVG_NUM (1 << AVG_FACT)

/*
 * There is only one unique Internal Sensor instance,
 * so use it privately.
 */
imon_t g_imon;

static void imon_convert()
{
	imon_t *imon = &g_imon;
	int16_t ts_data = *imon->adc_ts;
	int32_t itemp;

	/* Temp = Data * Slope - offset + 30 */
	itemp = (ts_data * imon->ts_slope) >> QFACT1;
	itemp = (itemp - imon->ts_offset) >> QFACT2;
	itemp += 30;

	imon->temp_degc = (int16_t)itemp;

	imon->vref =  (*VREFINT_CAL_ADDR) * VREFINT_CAL_VREF / (*imon->adc_vrefint);

	return;
}

void imon_init(int16_t *adc_ts, int16_t *adc_vrefint)
{
	imon_t *imon = &g_imon;

	imon->adc_ts = adc_ts;
	imon->adc_vrefint = adc_vrefint;

	uint16_t ts_cal1 = *TEMPSENSOR_CAL1_ADDR;
	uint16_t ts_cal2 = *TEMPSENSOR_CAL2_ADDR;

	imon->ts_slope = (110 << 16) / (ts_cal2 - ts_cal1); /* 110 = (130C - 30C) * 3.3V / 3V
	                                                 * Where:  130C calibration temp2;
	                                                 *          30C calibration temp1;
	                                                 *          3.3V VDDA
	                                                 *          3V calibration voltage
	                                                 */
	imon->ts_offset = (ts_cal1 * imon->ts_slope) >> QFACT1;
	imon->ts_offset  = (imon->ts_offset * 59578) >> 16;		/* 59578 = 3V/3.3V  (Q16) */

	/* Invalidate temperature */
	imon->temp_degc = INT16_MAX;
	imon->vref = INT16_MAX;

	imon->last_report_tick = HAL_GetTick();
}

/* Note: Called from ISR */
void imon_adc_completed ()
{
	imon_t *imon = &g_imon;
	imon->adc_ready = 1;
}

void imon_on_idle (uint32_t now_tick)
{
	imon_t *its = &g_imon;

	if (now_tick - its->last_report_tick > 1000 ){
		its->last_report_tick = now_tick;

		if (its->adc_ready) {
			imon_convert(its);
		}
	}

}

