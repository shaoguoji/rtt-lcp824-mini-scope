#include <rtthread.h>
#include <stdio.h>

#include "board.h"

#include "miniscope.h"
#include "drv_adc.h"
#include "menu.h"

struct Miniscope miniscope;

char *time_scale_text[TIME_SCALE_MAX_NUM] = TIME_SCALE_MENU_TEXT;
char *trig_dire_text[TRIG_DIRE_MAX_NUM] = TRIG_DIRE_MENU_TEXT;

int miniscope_init(void)
{
    /* miniscope menu init */
    miniscope.menu[MENU_TYPE_TIME_SCALE].type = MENU_TYPE_TIME_SCALE;
    miniscope.menu[MENU_TYPE_TIME_SCALE].value = 5000; /* defalut 5ms time scale */
    miniscope.menu[MENU_TYPE_TIME_SCALE].text = time_scale_text;

    miniscope.menu[MENU_TYPE_TRI_DIR].type = MENU_TYPE_TRI_DIR;
    miniscope.menu[MENU_TYPE_TRI_DIR].value = TRIG_DIRE_RISING;
    miniscope.menu[MENU_TYPE_TRI_DIR].text = trig_dire_text;

    miniscope.menu[MENU_TYPE_VOLT_SCALE].type = MENU_TYPE_VOLT_SCALE;
    miniscope.menu[MENU_TYPE_VOLT_SCALE].value = 3000; /* defalut 3V voltage scale*/
    miniscope.menu[MENU_TYPE_VOLT_SCALE].text = RT_NULL;

    /* miniscope adc init */
    miniscope.adc.channel = BOARD_ADC_CH3;
    miniscope.adc.buff = rt_malloc(ADC_SAMPLE_NUM*sizeof(rt_uint32_t));
    miniscope.adc.mb = rt_mb_create("adc_mb", 4, RT_IPC_FLAG_FIFO);
	if (miniscope.adc.mb == RT_NULL)
	{
		rt_kprintf("adc mailbox create faile.\n");
	}
    miniscope.adc.interval_us = SCALE_TO_INTERVAL(miniscope.menu[MENU_TYPE_TIME_SCALE].value);

    /* miniscope wave init */
    miniscope.wave.data = rt_malloc(WAVE_DATA_NUM*sizeof(rt_uint32_t));

    miniscope.key_event = rt_event_create("key_event", RT_IPC_FLAG_FIFO);
	if (miniscope.key_event == RT_NULL)
	{
		rt_kprintf("key event create faile.\r\n");
	}
}
INIT_APP_EXPORT(miniscope_init);
