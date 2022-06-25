#include "dev0.h"
#include "imon.h"
#include "cdc_ictrl.h"

void dev0_init ()
{
	dev0_t *dev0 = &g_dev0;

	dev0->last_report_tick = HAL_GetTick();
}

int g_dev0_dbg = 1;
void dev0_on_idle (uint32_t now_tick)
{
	static int presc_cnt = 0;
	int presc = ((presc_cnt++ & 0x0F) == 0);

	dev0_t *dev0 = &g_dev0;
	imon_t *imon = &g_imon;

	if (now_tick - dev0->last_report_tick > 100 ){
		dev0->last_report_tick = now_tick;

		if (dev0->hid && imon->temp_degc != INT16_MAX) {
			dev0->hid_in_report.temperature = (int8_t)imon->temp_degc;
			dev0->hid_in_report.voltage = (uint8_t)((imon->vref + 50) / 100) ;
			HID_SendReport(dev0->hid, (uint8_t*)&dev0->hid_in_report,
					sizeof(dev0->hid_in_report));
		}
		if (presc && (imon->temp_degc != INT16_MAX) && g_dev0_dbg == 1){
			static int cnt = 0;
			ictrl_printf_nonisr("[%d] %dC, %dmV\r\n",
					cnt++, imon->temp_degc, imon->vref);
		}
	}
}
