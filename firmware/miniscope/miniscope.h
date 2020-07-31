#ifndef __MINISCOPE_H__
#define __MINISCOPE_H__

#include <rtthread.h>
#include "menu.h"

#define ADC_CONVERT_PERIOD_US   1   /* 1M ADC Convert Rate */

#define ADC_MAX_VOLT    3300
#define ADC_MIN_VOLT    0

#define ADC_SAMPLE_NUM          100
#define WAVE_DATA_NUM           100

#define SCALE_TO_INTERVAL(s)    ((s)*4/ADC_SAMPLE_NUM)

struct Adc_Info
{
    rt_uint32_t channel;
    rt_uint16_t *buff;
    rt_mailbox_t mb;
    rt_uint32_t interval_us;
};

struct Wave_Info
{
    rt_uint32_t *data;
    rt_mailbox_t mb;
    rt_uint16_t vMax;       /* mv */
    rt_uint16_t vMin;
    rt_uint16_t rulerVMax;
    rt_uint16_t rulerVMin;
};

struct Miniscope 
{
    struct Adc_Info adc;
    struct Wave_Info wave;
    struct Menu_Info menu[MENU_TYPE_MAX_NUM];

    rt_uint16_t option_index;
    rt_uint16_t tri_pos;
    rt_bool_t trig_dire;
    rt_err_t trigger_state;
    rt_event_t key_event;
};

#endif /* #ifndef __MINISCOPE_H__ */
