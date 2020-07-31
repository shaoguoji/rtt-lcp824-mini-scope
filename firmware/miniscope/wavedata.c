#include <rtthread.h>
#include <rthw.h>
#include <stdio.h>

#include "board.h"

#include "drv_adc.h"
#include "miniscope.h"
#include "menu.h"

extern struct Miniscope miniscope;

/* 将采样值的映射到屏幕的显示范围，并反转
   Remap sampling data to display range and inverse */
rt_uint16_t remap(rt_uint16_t val, rt_uint16_t rangeMax, rt_uint16_t rangeMin, rt_uint16_t rangeMaxNew, rt_uint16_t rangeMinNew)
{
    if (val > rangeMax)
        val = rangeMax;
    else if (val < rangeMin)
        val = rangeMin;

    val = rangeMinNew + (rt_uint32_t)(rangeMax - val) * (rangeMaxNew - rangeMinNew) / (rangeMax - rangeMin);
    return val;
}

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

void data_parse_entry(void *parameter)
{
    rt_uint16_t *adc_buff = RT_NULL;
    rt_uint16_t tmp = 0;
    rt_uint16_t dacMax = 0, dacMin = 4095;
    rt_uint16_t plotADCMax = 0, plotADCMin = 0;
	int i;
	
    while (1)
	{
		if (rt_mb_recv(miniscope.adc.mb, (rt_uint32_t *)&adc_buff, RT_WAITING_FOREVER) == RT_EOK)
		{
			for (i = 0; i < ADC_SAMPLE_NUM; i++)
            {
                tmp = adc_buff[i];
                if (tmp > dacMax)
                    dacMax = tmp;
                else if (tmp < dacMin)
                    dacMin = tmp;
            }

            //将采样点的最大最小ADC值转换成电压值mV
            miniscope.wave.vMax = (rt_uint32_t)dacMax * 3300 / 4096;
            miniscope.wave.vMin = (rt_uint32_t)dacMin * 3300 / 4096;

            if(miniscope.menu[MENU_TYPE_VOLT_SCALE].index == VOLT_SCALE_Auto)
            {
                //根据采样点的最大最小值，按500mV扩大范围取整，作为垂直标尺范围mV
                if (miniscope.wave.vMax / 100 % 10 >= 5)
                    miniscope.wave.rulerVMax = (miniscope.wave.vMax + 500) / 1000 * 1000;
                else
                    miniscope.wave.rulerVMax = miniscope.wave.vMax / 1000 * 1000 + 500;
                if (miniscope.wave.vMin / 100 % 10 >= 5)
                    miniscope.wave.rulerVMin = miniscope.wave.vMin / 1000 * 1000 + 500;
                else
                    miniscope.wave.rulerVMin = miniscope.wave.vMin / 1000 * 1000;

                if (miniscope.wave.rulerVMax > ADC_MAX_VOLT)
                    miniscope.wave.rulerVMax = ADC_MAX_VOLT;
                if (miniscope.wave.rulerVMin < ADC_MIN_VOLT)
                    miniscope.wave.rulerVMin = ADC_MIN_VOLT;
            }
            else
            {
                // 根据手动量程计算垂直标尺最大值mV
                miniscope.wave.rulerVMin = 0;
                switch(miniscope.menu[MENU_TYPE_VOLT_SCALE].index)
                {
                    case VOLT_SCALE_200MV:
                        miniscope.wave.rulerVMax = 200;
                        break;
                    case VOLT_SCALE_500MV:
                        miniscope.wave.rulerVMax = 500;
                        break;
                    case VOLT_SCALE_1V:
                        miniscope.wave.rulerVMax = 1000;
                        break;
                    case VOLT_SCALE_2V:
                        miniscope.wave.rulerVMax = 2000;
                        break;
                    case VOLT_SCALE_3V:
                        miniscope.wave.rulerVMax = 3000;
                        break;
                    default:
                        break;			
                }
            }
            //用垂直标尺mV范围反求出ADC值的范围作为图表的显示上下限
            plotADCMax = (rt_uint32_t)miniscope.wave.rulerVMax * 4096 / 3300;
            plotADCMin = (rt_uint32_t)miniscope.wave.rulerVMin * 4096 / 3300;


            //重新映射采样点ADC值至图表的垂直范围
            for (i = 0; i < ADC_SAMPLE_NUM; i++)
            {
                //printf("%hd\r\n", *(WaveData + i));
                miniscope.wave.data[i] = remap(adc_buff[i], plotADCMax, plotADCMin, 52, 1);
            }
            rt_mb_send_wait(miniscope.wave.mb, (rt_uint32_t)&miniscope.wave.data[0], RT_WAITING_FOREVER);
		}
	}
}

int wavedata_init(void)
{
    rt_thread_t adc_thread = RT_NULL;
    rt_thread_t parse_thread = RT_NULL;

    adc_thread = rt_thread_create("adc_thread", adc_sample_entry, RT_NULL, 256, 4, 30);
    if (adc_thread != RT_NULL)
    {
        rt_thread_startup(adc_thread);
    }

    parse_thread = rt_thread_create("parse_thread", data_parse_entry, RT_NULL, 256, 5, 30);
    if (parse_thread != RT_NULL)
    {
        rt_thread_startup(parse_thread);
    }
}
INIT_APP_EXPORT(wavedata_init);


