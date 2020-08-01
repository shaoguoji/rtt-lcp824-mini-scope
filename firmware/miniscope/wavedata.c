#include <rtthread.h>
#include <rthw.h>
#include <stdio.h>

#include "board.h"

#include "drv_adc.h"
#include "miniscope.h"
#include "menu.h"

extern struct Miniscope miniscope;
rt_thread_t adc_thread = RT_NULL;

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
	int i, t;

	while (1)
	{
        t = miniscope.menu[MENU_TYPE_TIME_SCALE].value[miniscope.menu[MENU_TYPE_TIME_SCALE].index];
        miniscope.adc.interval_us = SCALE_TO_INTERVAL(t);

        // rt_enter_critical();
        for (i = 0; i < ADC_SAMPLE_NUM; i++)
        {
            lpc824_get_adc_value(miniscope.adc.channel, &miniscope.adc.buff[i]);
            rt_hw_us_delay(miniscope.adc.interval_us - ADC_CONVERT_PERIOD_US); // us delay
        }
        // rt_exit_critical();

        rt_mb_send_wait(miniscope.adc.mb, (rt_uint32_t)&miniscope.adc.buff[0], RT_WAITING_FOREVER);
	}
}

void data_parse_entry(void *parameter)
{
    rt_uint16_t *adc_buff = RT_NULL;
    rt_uint16_t tmp = 0;
    rt_uint16_t dacMax = 0, dacMin = 4095, dacMid = 0;
    rt_uint16_t plotADCMax = 0, plotADCMin = 0;
	int i, option_index;
	
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
                option_index = miniscope.menu[MENU_TYPE_VOLT_SCALE].index;
                miniscope.wave.rulerVMax = miniscope.menu[MENU_TYPE_VOLT_SCALE].value[option_index];
            }
            //用垂直标尺mV范围反求出ADC值的范围作为图表的显示上下限
            plotADCMax = (rt_uint32_t)miniscope.wave.rulerVMax * 4096 / 3300;
            plotADCMin = (rt_uint32_t)miniscope.wave.rulerVMin * 4096 / 3300;

            //查询触发位置
            dacMid = (dacMax + dacMin) >> 1;
            miniscope.tri_pos = ADC_SAMPLE_NUM / 2;

            for (i = 50; i <= ADC_SAMPLE_NUM-50; i++)
            {
                if (miniscope.menu[MENU_TYPE_TRI_DIR].index == TRIG_DIRE_RISING) //上升触发
                {
                    if (adc_buff[i] <= dacMid && adc_buff[i+1] >= dacMid)
                    {
                        miniscope.tri_pos = i;
                        break;
                    }
                }
                else //下降触发
                {
                    if (adc_buff[i] >= dacMid && adc_buff[i+1] <= dacMid)
                    {
                        miniscope.tri_pos = i;
                        break;
                    }
                }
            }

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
    rt_thread_t parse_thread = RT_NULL;

    adc_thread = rt_thread_create("adc_thread", adc_sample_entry, RT_NULL, ADC_THREAD_STACK_SIZE, ADC_THREAD_PRIO, 30);
    if (adc_thread != RT_NULL)
    {
        rt_thread_startup(adc_thread);
    }

    parse_thread = rt_thread_create("parse_thread", data_parse_entry, RT_NULL, PARSE_THREAD_STACK_SIZE, PARSE_THREAD_PRIO, 30);
    if (parse_thread != RT_NULL)
    {
        rt_thread_startup(parse_thread);
    }
}
INIT_APP_EXPORT(wavedata_init);


