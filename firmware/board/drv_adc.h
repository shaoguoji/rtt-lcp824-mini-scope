#ifndef __DRV_ADC_H__
#define __DRV_ADC_H__

#define BOARD_ADC_CH1 2
#define BOARD_ADC_CH2 10
#define BOARD_ADC_CH3 9

#define EVENT_FLAG_COMPLETE (1 << 0)
#define EVENT_FLAG_THRESHOLD (1 << 1)

void lpc824_get_adc_value(rt_uint32_t channel, rt_uint16_t *value);
void Board_ADC_Start(void);
void Board_ADC_Stop(void);

#endif /* #ifndef __DRV_ADC_H__ */
