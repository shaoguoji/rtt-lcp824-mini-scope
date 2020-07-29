#ifndef __MENU_H__
#define __MENU_H__

#include <rtthread.h>

#define TIME_SCALE_MENU_TEXT {"100ms", "50ms", "20ms", "10ms", "5ms", "2ms", "1ms", "500us", "200us", "100us"}
#define TRIG_DIRE_MENU_TEXT {"R", "F"};

enum MENU_TYPE_LIST 
{ 
    MENU_TYPE_TIME_SCALE = 0, 
    MENU_TYPE_TRI_DIR, 
    MENU_TYPE_VOLT_SCALE,
    /* end of enum */
    MENU_TYPE_MAX_NUM
};

enum MENU_TIME_SCALE_VALUE 
{ 
    TIME_SCALE_100MS = 0, 
    TIME_SCALE_50MS, 
    TIME_SCALE_20MS, 
    TIME_SCALE_10MS, 
    TIME_SCALE_5MS, 
    TIME_SCALE_2MS, 
    TIME_SCALE_1MS, 
    TIME_SCALE_500US, 
    TIME_SCALE_200US, 
    TIME_SCALE_100US, 
    /* end of enum */
    TIME_SCALE_MAX_NUM
};

enum MENU_TRIG_DIRE_VALUE 
{ 
    TRIG_DIRE_RISING = 0,
    TRIG_DIRE_FALLING,
    /* end of enum */
    TRIG_DIRE_MAX_NUM
};

struct Menu_Info 
{
    enum MENU_TYPE_LIST type;
    rt_uint32_t value;
    char **text;
};

#endif /* #ifndef __MENU_H__ */
