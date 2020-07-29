#include <rtthread.h>
#include <rthw.h>
#include <stdio.h>

#include "board.h"

#include "drv_adc.h"
#include "miniscope.h"
#include "menu.h"

extern struct Miniscope miniscope;

void adc_sample_entry(void *parameter)
{
	rt_uint32_t v = 0;
	int i;

	while (1)
	{
        for (i = 0; i < ADC_SAMPLE_NUM; i++)
        {
            lpc824_get_adc_value(miniscope.adc.channel, &miniscope.adc.buff[i]);
            rt_hw_us_delay(miniscope.adc.interval_us - ADC_CONVERT_PERIOD_US); // us delay
        }
        rt_mb_send_wait(miniscope.adc.mb, (rt_uint32_t)&miniscope.adc.buff[0], RT_WAITING_FOREVER);
	}
}

int wavedata_init(void)
{
    rt_thread_t adc_thread = RT_NULL;

    adc_thread = rt_thread_create("adc_thread", adc_sample_entry, RT_NULL, 512, 2, 30);
    if (adc_thread != RT_NULL)
    {
        rt_thread_startup(adc_thread);
    }
}
INIT_APP_EXPORT(wavedata_init);


