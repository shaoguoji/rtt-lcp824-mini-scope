#include <rtthread.h>

#include <stdio.h>
#include "board.h"

/**
 * @brief	main routine for template example
 * @return	Function should not exit.
 */
int main(void)
{
	SystemCoreClockUpdate();
	
	rt_kprintf("Hello RT-Thread!\r\n");
	
	/* LED Pin Init */
	Chip_GPIO_PinSetDIR(LPC_GPIO_PORT, 0, 28, 1);
    Chip_GPIO_PinSetState(LPC_GPIO_PORT, 0, 28, true);

	while(1)
    {
		rt_thread_mdelay(1000);
	}
}
