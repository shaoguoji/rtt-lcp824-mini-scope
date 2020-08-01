#include <rtthread.h>
#include <stdio.h>

#include "board.h"

#include "miniscope.h"
#include "drv_adc.h"
#include "menu.h"

struct Miniscope miniscope;

static char *menu_time_scale_text[TIME_SCALE_MAX_NUM] = TIME_SCALE_MENU_TEXT;
static char *menu_trig_dire_text[TRIG_DIRE_MAX_NUM] = TRIG_DIRE_MENU_TEXT;
static char *menu_volt_scale_text[VOLT_SCALE_MAX_NUM] = VOLT_SCALE_MENU_TEXT;

static rt_uint32_t menu_time_scale_value[TIME_SCALE_MAX_NUM] = TIME_SCALE_MENU_VALUE;
static rt_uint32_t menu_trig_dire_value[TRIG_DIRE_MAX_NUM] = TRIG_DIRE_MENU_VALUE;
static rt_uint32_t menu_volt_scale_value[VOLT_SCALE_MAX_NUM] = VOLT_SCALE_MENU_VALUE;

int miniscope_init(void)
{
    /* miniscope menu init */
    struct Menu_Info time_scale = { MENU_TYPE_TIME_SCALE, 
                                    TIME_SCALE_5MS, 
                                    menu_time_scale_value, 
                                    TIME_SCALE_MAX_NUM,
                                    menu_time_scale_text };

    struct Menu_Info trig_dire = {  MENU_TYPE_TRI_DIR, 
                                    TRIG_DIRE_RISING, 
                                    menu_trig_dire_value, 
                                    TRIG_DIRE_MAX_NUM,
                                    menu_trig_dire_text };

    struct Menu_Info volt_scale = { MENU_TYPE_VOLT_SCALE, 
                                    VOLT_SCALE_Auto, 
                                    menu_volt_scale_value,
                                    VOLT_SCALE_MAX_NUM,
                                    menu_volt_scale_text };

    miniscope.menu[MENU_TYPE_TIME_SCALE] = time_scale;
    miniscope.menu[MENU_TYPE_TRI_DIR] = trig_dire;
    miniscope.menu[MENU_TYPE_VOLT_SCALE] = volt_scale;

    /* miniscope adc init */
    miniscope.adc.channel = BOARD_ADC_CH3;
    miniscope.adc.buff = rt_malloc(ADC_SAMPLE_NUM*sizeof(rt_uint32_t));
    miniscope.adc.mb = rt_mb_create("adc_mb", 4, RT_IPC_FLAG_FIFO);
	if (miniscope.adc.mb == RT_NULL)
	{
		rt_kprintf("adc mailbox create faile.\n");
	}
    miniscope.adc.interval_us = SCALE_TO_INTERVAL(miniscope.menu[MENU_TYPE_TIME_SCALE].value[miniscope.menu[MENU_TYPE_TIME_SCALE].index]);

    /* miniscope wave init */
    miniscope.wave.data = rt_malloc(WAVE_DATA_NUM*sizeof(rt_uint32_t));
    miniscope.wave.mb = rt_mb_create("wave_mb", 4, RT_IPC_FLAG_FIFO);

    miniscope.option_index = 0;
    miniscope.key_event = rt_event_create("key_event", RT_IPC_FLAG_FIFO);
	if (miniscope.key_event == RT_NULL)
	{
		rt_kprintf("key event create faile.\r\n");
	}
}
INIT_APP_EXPORT(miniscope_init);
