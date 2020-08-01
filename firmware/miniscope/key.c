#include <rtthread.h>

#include "drv_adc.h"

#include "key.h"
#include "menu.h"
#include "miniscope.h"

extern struct Miniscope miniscope;
extern rt_thread_t adc_thread;

static void key_timeout(void *parameter)
{
	rt_uint32_t temp = 0x00;
	static rt_uint32_t hold_time;
	static rt_uint32_t key_map;

	temp = ((!LPC824_READ_PIN(PIN_KEY_LEFT)<<PIN_KEY_LEFT) | (!LPC824_READ_PIN(PIN_KEY_RIGHT)<<PIN_KEY_RIGHT) | 
			(!LPC824_READ_PIN(PIN_KEY_UP)<<PIN_KEY_UP) | (!LPC824_READ_PIN(PIN_KEY_DOWN)<<PIN_KEY_DOWN) | 
			(!LPC824_READ_PIN(PIN_KEY_OK)<<PIN_KEY_OK));

	// rt_kprintf("tem = %08x\n", temp);
	if (temp != 0) // any key press hold, time count
	{
		hold_time++;
		key_map = temp;
	}
	else  // all key release, get result
	{
		if ((hold_time > KEY_SHAKE_TIME) && (hold_time < KEY_hold_time_MAX))
		{
			/* press event */
			rt_event_send(miniscope.key_event, key_map & EVENT_KEY_LEFT_PRESS);
			rt_event_send(miniscope.key_event, key_map & EVENT_KEY_RIGHT_PRESS);
			rt_event_send(miniscope.key_event, key_map & EVENT_KEY_UP_PRESS);
			rt_event_send(miniscope.key_event, key_map & EVENT_KEY_DOWN_PRESS);
			rt_event_send(miniscope.key_event, key_map & EVENT_KEY_OK_PRESS);

			key_map = 0x00;
		}
		// else if (hold_time > KEY_LONG_hold_time_MIN)
		// {
		// 	/* long press event */
		// }
		hold_time = 0;
	}
}

void menu_thread_entry(void *parameter)
{
	rt_uint32_t e;
    int option_count = 0;
    static int onoff_toggle = 0;

	while (1)
	{
		if ((rt_event_recv(miniscope.key_event, (EVENT_KEY_LEFT_PRESS | EVENT_KEY_RIGHT_PRESS | EVENT_KEY_UP_PRESS | 
		                   EVENT_KEY_DOWN_PRESS | EVENT_KEY_OK_PRESS), (RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR), 
						   RT_WAITING_FOREVER, &e) == RT_EOK))
		{
			if ((e & EVENT_KEY_LEFT_PRESS) != 0) 
            { 
                // rt_kprintf("key LIFT press!\n"); 
                miniscope.option_index = (miniscope.option_index + 1) % MENU_TYPE_MAX_NUM;
            }

			if ((e & EVENT_KEY_RIGHT_PRESS) != 0) 
            { 
                // rt_kprintf("key RIGHT press!\n"); 
                option_count = miniscope.menu[miniscope.option_index].count;

                if (miniscope.menu[miniscope.option_index].index > 0)
                {
                    miniscope.menu[miniscope.option_index].index--;
                }
                else
                {
                    miniscope.menu[miniscope.option_index].index = option_count-1;
                }
            }

			if ((e & EVENT_KEY_UP_PRESS) != 0) 
            { 
                // rt_kprintf("key UP press!\n"); 
                option_count = miniscope.menu[miniscope.option_index].count;
                miniscope.menu[miniscope.option_index].index++;
                miniscope.menu[miniscope.option_index].index %= option_count;
            }

			if ((e & EVENT_KEY_DOWN_PRESS) != 0) 
            { 
                // rt_kprintf("key DOWN press!\n"); 
                if (miniscope.adc.channel == BOARD_ADC_CH3)
                    miniscope.adc.channel = BOARD_ADC_CH1;
                else
                    miniscope.adc.channel = BOARD_ADC_CH3;
            }

			if ((e & EVENT_KEY_OK_PRESS) != 0) 
            { 
                // rt_kprintf("key OK press!\n"); 
                if (++onoff_toggle % 2 == 0)
                {
                    rt_thread_resume(adc_thread);
                    Board_ADC_Start();
                }
                else
                {
                    rt_thread_suspend(adc_thread);
                    Board_ADC_Stop();
                }
            }
		}
	}
}

/**
 * @brief	main routine for template example
 * @return	Function should not exit.
 */
static int key_init(void)
{
	rt_timer_t key_timer = RT_NULL;
    rt_thread_t menu_thread = RT_NULL;

	Chip_GPIO_PinSetDIR(LPC_GPIO_PORT, 0, PIN_KEY_LEFT, 0);
	Chip_GPIO_PinSetDIR(LPC_GPIO_PORT, 0, PIN_KEY_RIGHT, 0);
	Chip_GPIO_PinSetDIR(LPC_GPIO_PORT, 0, PIN_KEY_UP, 0);
	Chip_GPIO_PinSetDIR(LPC_GPIO_PORT, 0, PIN_KEY_DOWN, 0);
	Chip_GPIO_PinSetDIR(LPC_GPIO_PORT, 0, PIN_KEY_OK, 0);

	key_timer = rt_timer_create("key_timer", key_timeout, RT_NULL, rt_tick_from_millisecond(10), RT_TIMER_FLAG_PERIODIC);
	if (key_timer != RT_NULL)
	{
		rt_timer_start(key_timer);
	}

	menu_thread = rt_thread_create("menu_thread", menu_thread_entry, RT_NULL, MENU_THREAD_STACK_SIZE, MENU_THREAD_PRIO, 30);
	if (menu_thread != RT_NULL)
	{
		rt_thread_startup(menu_thread);
	}
}
INIT_APP_EXPORT(key_init);
