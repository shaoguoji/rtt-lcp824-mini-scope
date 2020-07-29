#include <rtthread.h>
#include <stdio.h>

#include "board.h"

#include "miniscope.h"
#include "oled.h"

#define DIS_THREAD_STACK_SIZE   512
static struct rt_thread dis_thread;
static rt_uint8_t dis_thread_stack[DIS_THREAD_STACK_SIZE];
static struct rt_timer refresh_timer;

extern struct Miniscope miniscope;

/* 屏幕刷新定时器 */
static void refresh_timeout(void *parameter)
{
    OLED_Refresh();
}

/* 显存更新线程入口 */
void dis_thread_entry(void *parameter)
{
	rt_uint16_t tempVal = 0;
	rt_uint32_t adc_value = 0;
	
	while (1)
	{
		if (rt_mb_recv(miniscope.adc.mb, &adc_value, RT_WAITING_FOREVER) == RT_EOK)
		{
			tempVal = (uint16_t)(((rt_uint16_t *)adc_value)[0]&0x0000007f)+40;
        	OLED_DrawPoint(127,((uint8_t)(tempVal*63/128)&0x3f),1);
			OLED_Left_Shift_Pixel();
		}
		rt_thread_mdelay(1);
	}
}

int display_init(void)
{
	rt_timer_init(&refresh_timer, "timer1", refresh_timeout, RT_NULL, rt_tick_from_millisecond(40), RT_TIMER_FLAG_PERIODIC);
	rt_thread_init(&dis_thread, "dis_thread", dis_thread_entry, RT_NULL, dis_thread_stack, DIS_THREAD_STACK_SIZE, 3, 10);

    rt_timer_start(&refresh_timer);
    rt_thread_startup(&dis_thread);
}
INIT_APP_EXPORT(display_init);
