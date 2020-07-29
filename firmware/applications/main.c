#include <rtthread.h>

#include <stdio.h>
#include "board.h"

#include "drv_adc.h"

/**
 * @brief	main routine for template example
 * @return	Function should not exit.
 */
int main(void)
{
	SystemCoreClockUpdate();
	
	rt_kprintf("Hello RT-Thread!\r\n");
	
	while(1)
    {
		rt_thread_mdelay(1000);
	}
}
