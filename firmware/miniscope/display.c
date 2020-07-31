#include <rtthread.h>
#include <stdio.h>

#include "board.h"

#include "miniscope.h"
#include "oled.h"

/* Region for waveform on display*/
#define CHART_H_MAX 126 
#define CHART_H_MIN 26
#define CHART_V_MAX 52
#define CHART_V_MIN 8

#define DIS_THREAD_STACK_SIZE   256
static struct rt_thread dis_thread;
static rt_uint8_t dis_thread_stack[DIS_THREAD_STACK_SIZE];
static struct rt_timer refresh_timer;

extern struct Miniscope miniscope;

rt_uint8_t *convertVol(rt_uint16_t v)
{
    static rt_uint8_t s[5];
    if (v < 10000)
    {
        s[0] = v / 1000 + '0';
        s[1] = '.';
        s[2] = v / 100 % 10 + '0';
        s[3] = v / 10 % 10 + '0';
        s[4] = '\0';
    }
    else
    {
        s[0] = v / 10000 + '0';
        s[1] = v / 1000 % 10 + '0';
        s[2] = '.';
        s[3] = v / 100 % 10 + '0';
        s[4] = '\0';
    }
    return s;
}

/* 屏幕刷新定时器 */
static void refresh_timeout(void *parameter)
{
    OLED_Display();
}

/* 绘制主界面
   Draw main interface */
void PlotChart(void)
{
    rt_uint8_t i;
    rt_uint8_t *s;
    
	OLED_Clear();

	//绘制图表实线边框
	OLED_DrawHLine(26, 0, 4); //方框内纵向留52格,1~52
	//OLED_DrawHLine(23, 26, 3);
	OLED_DrawHLine(26, 53, 4);
	
	OLED_DrawHLine(123, 0, 4);
	OLED_DrawHLine(123, 53, 4);
	OLED_DrawVLine(25, 0, 54); //方框内横向留101格,26~126
	OLED_DrawVLine(127, 0, 54);

	//绘制图表虚线网格
	for (i = 0; i < 17; i++)
	{
		OLED_DrawHLine(28 + 6 * i, 26, 2);
	}
	for (i = 0; i < 9; i++)
	{
		OLED_DrawVLine(26 + 25, 2 + i * 6, 2);
		OLED_DrawVLine(26 + 50, 2 + i * 6, 2);
		OLED_DrawVLine(26 + 75, 2 + i * 6, 2);
	}

	//绘制触发方式标志
	if (miniscope.menu[MENU_TYPE_TRI_DIR].index == TRIG_DIRE_RISING)
	{
		OLED_DrawChar(18, 56, 123); //123上箭头，上升沿触发
	}
	else
	{
		OLED_DrawChar(18, 56, 124); //124下箭头，下降沿触发
	}
	
	//绘制波形电压范围
	OLED_Set_Pos(26, 56);
	s = convertVol(miniscope.wave.vMin);
	OLED_DrawString(s);
	OLED_DrawString("-");
	s = convertVol(miniscope.wave.vMax);
	OLED_DrawString(s);
	OLED_DrawString("V");
	
	// //绘制触发失败标志
	// // if (TriggerFail)
	// // {
	// // 	OLED_Set_Pos(0, 16);
	// // 	OLED_DrawString("Tri.");
	// // 	OLED_Set_Pos(0, 24);
	// // 	OLED_DrawString("Fail");
	// // }

	//绘制自动量程标志
	// if(HoldDisplay)
	// {
	// 	OLED_Overlap(1);
	// 	OLED_Set_Pos(8, 56);
	// 	OLED_DrawString(" ");
	// 	OLED_Overlap(0);	
	// }
	if(miniscope.menu[MENU_TYPE_VOLT_SCALE].index == VOLT_SCALE_Auto)
	{
		OLED_Set_Pos(8, 56);
		OLED_DrawString("A");
	}

	//绘制横轴时间区间
	// if(HoldDisplay)
	// {
	// 	OLED_Overlap(1);
	// 	OLED_Set_Pos(97, 56);
	// 	OLED_DrawString("     ");
	// 	OLED_Overlap(0);	
	// }
	if(miniscope.option_index == MENU_TYPE_TIME_SCALE)
	{
		OLED_Reverse(1);
	}
	OLED_Set_Pos(97, 56);
	OLED_DrawString(miniscope.menu[MENU_TYPE_TIME_SCALE].text[miniscope.menu[MENU_TYPE_TIME_SCALE].index]);
	OLED_Reverse(0);
	
	//绘制纵轴电压区间
	// if(HoldDisplay)
	// {
	// 	OLED_Overlap(1);
	// 	OLED_Set_Pos(0, 0);
	// 	OLED_DrawString("    ");
	// 	OLED_Overlap(0);
	// }		
	if(miniscope.option_index == MENU_TYPE_VOLT_SCALE)
	{
		OLED_Reverse(1);
	}
	s = convertVol(miniscope.wave.rulerVMax);
	OLED_Set_Pos(0, 0);
	OLED_DrawString(s);
	OLED_Reverse(0);
	
	s = convertVol(miniscope.wave.rulerVMin);
	OLED_Set_Pos(0, 45);
	OLED_DrawString(s);
		
    //绘制等待标志
    // if (WaitADC)
    // {
    //     OLED_Set_Pos(0, 8);
    //     OLED_DrawString("Wait");
    // }
    // else
    // {
    //     OLED_Overlap(0); //设置绘图模式为覆盖
    //     OLED_Set_Pos(0, 8);
    //     OLED_DrawString("    ");
    //     OLED_Overlap(1); //恢复绘图模式为叠加
    // }
    // //绘制Hold标志
    // if (HoldDisplay)
    // {
    //     OLED_Set_Pos(0, 56);
    //     OLED_DrawString("H");
    // }
    // else
    // {
        OLED_Overlap(0); //设置绘图模式为覆盖
        OLED_Set_Pos(0, 56);
        OLED_DrawString(" ");
        OLED_Overlap(1); //恢复绘图模式为叠加
    // }
}

/* 显存更新线程入口 */
void dis_thread_entry(void *parameter)
{
	rt_uint16_t tempVal = 0;
	rt_uint32_t *dis_buff = RT_NULL;
	int i;

	while (1)
	{
		if (rt_mb_recv(miniscope.wave.mb, (rt_uint32_t *)&dis_buff, RT_WAITING_FOREVER) == RT_EOK)
		{
			PlotChart();
			for (i = 0; i < ADC_SAMPLE_NUM-1; i++)
			{
				OLED_DrawLine(i+26, dis_buff[i], i+26+1, dis_buff[i+1]);
			}
		}
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
