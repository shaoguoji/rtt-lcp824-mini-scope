/*
 * @brief LPC82x ADC example
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2013
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include <rtthread.h>
#include "board.h"

#include "drv_adc.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

// static bool sequenceComplete;
static struct rt_event adc_seqa_event;
// static bool thresholdCrossed;

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/

// Chip_ADC_StartSequencer(LPC_ADC, ADC_SEQA_IDX);

/**
 * @brief	Handle interrupt from ADC sequencer A
 * @return	Nothing
 */
void ADC_SEQA_IRQHandler(void)
{
	uint32_t pending;

	/* Get pending interrupts */
	pending = Chip_ADC_GetFlags(LPC_ADC);

	/* Sequence A completion interrupt */
	if (pending & ADC_FLAGS_SEQA_INT_MASK) {
		// sequenceComplete = true;
		rt_event_send(&adc_seqa_event, EVENT_FLAG_COMPLETE);
	}

	/* Threshold crossing interrupt on ADC input channel */
	// if (pending & ADC_FLAGS_THCMP_MASK(BOARD_ADC_CH)) {
	// 	thresholdCrossed = true;
	// }

	/* Clear any pending interrupts */
	Chip_ADC_ClearFlags(LPC_ADC, pending);
}

/**
 * @brief	main routine for ADC example
 * @return	Function should not exit
 */
void _hw_lpc824_adc_init(rt_uint32_t channel)
{
	static rt_uint32_t option;
	
	/* Setup ADC for 12-bit mode and normal power */
	Chip_ADC_Init(LPC_ADC, 0);

	/* Need to do a calibration after initialization and trim */
	Chip_ADC_StartCalibration(LPC_ADC);
	while (!(Chip_ADC_IsCalibrationDone(LPC_ADC))) {}

	/* Setup for maximum ADC clock rate using sycnchronous clocking */
	Chip_ADC_SetClockRate(LPC_ADC, ADC_MAX_SAMPLE_RATE);

	/* Optionally, you can setup the ADC to use asycnchronous clocking mode.
	   To enable this, mode use 'LPC_ADC->CTRL |= ADC_CR_ASYNMODE;'.
	   In asycnchronous clocking mode mode, the following functions are
	   used to set and determine ADC rates:
	   Chip_Clock_SetADCASYNCSource();
	   Chip_Clock_SetADCASYNCClockDiv();
	   Chip_Clock_GetADCASYNCRate();
	   clkRate = Chip_Clock_GetADCASYNCRate() / Chip_Clock_GetADCASYNCClockDiv; */

	/* Setup sequencer A for ADC channel 3, EOS interrupt */
	/* Setup a sequencer to do the following:
	   Perform ADC conversion of ADC channels 3 only */
	option |= (ADC_SEQ_CTRL_CHANSEL(channel) | ADC_SEQ_CTRL_MODE_EOS);
	Chip_ADC_SetupSequencer(LPC_ADC, ADC_SEQA_IDX, option);

	/* Enable the clock to the Switch Matrix */
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SWM);

	/* Configure the SWM for P0-23 as the input for the ADC1 */
	Chip_SWM_EnableFixedPin(SWM_FIXED_ADC0+channel);

	/* Disable the clock to the Switch Matrix to save power */
	Chip_Clock_DisablePeriphClock(SYSCTL_CLOCK_SWM);


	/* Setup threshold 0 low and high values to about 25% and 75% of max */
	// Chip_ADC_SetThrLowValue(LPC_ADC, 0, ((1 * 0xFFF) / 4));
	// Chip_ADC_SetThrHighValue(LPC_ADC, 0, ((3 * 0xFFF) / 4));

	/* Clear all pending interrupts */
	Chip_ADC_ClearFlags(LPC_ADC, Chip_ADC_GetFlags(LPC_ADC));

	/* Enable ADC overrun and sequence A completion interrupts */
	Chip_ADC_EnableInt(LPC_ADC, (ADC_INTEN_SEQA_ENABLE | ADC_INTEN_OVRRUN_ENABLE));

	/* Use threshold 0 for ADC channel and enable threshold interrupt mode for
	   channel as crossing */
	// Chip_ADC_SelectTH0Channels(LPC_ADC, ADC_THRSEL_CHAN_SEL_THR1(channel));
	// Chip_ADC_SetThresholdInt(LPC_ADC, channel, ADC_INTEN_THCMP_CROSSING);

	/* Enable ADC NVIC interrupt */
	NVIC_EnableIRQ(ADC_SEQA_IRQn);

	/* Enable sequencer */
	Chip_ADC_EnableSequencer(LPC_ADC, ADC_SEQA_IDX);
}

static int lpc824_adc_init(void)
{
	rt_err_t result;

	result = rt_event_init(&adc_seqa_event, "adc_event", RT_IPC_FLAG_FIFO);
	if (result != RT_EOK)
    {
        rt_kprintf("init event failed.\n");
        return -RT_ERROR;
    }

	_hw_lpc824_adc_init(BOARD_ADC_CH1);
	_hw_lpc824_adc_init(BOARD_ADC_CH2);
	_hw_lpc824_adc_init(BOARD_ADC_CH3);
}
INIT_BOARD_EXPORT(lpc824_adc_init);

void lpc824_get_adc_value(rt_uint32_t channel, rt_uint16_t *value)
{
	rt_uint32_t e;
	rt_uint32_t raw_sample;

	Chip_ADC_StartSequencer(LPC_ADC, ADC_SEQA_IDX);

	if (rt_event_recv(&adc_seqa_event, EVENT_FLAG_COMPLETE,
                      RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                      RT_WAITING_FOREVER, &e) == RT_EOK)
    {
		raw_sample = Chip_ADC_GetDataReg(LPC_ADC, channel);
		if (raw_sample & (ADC_DR_OVERRUN | ADC_SEQ_GDAT_DATAVALID)) 
		{
			*value =  ADC_DR_RESULT(raw_sample);
		}
	}
}

void Board_ADC_Stop(void)
{
    Chip_ADC_DisableSequencer(LPC_ADC, ADC_SEQA_IDX);
}

void Board_ADC_Start(void)
{
    Chip_ADC_EnableSequencer(LPC_ADC, ADC_SEQA_IDX);
    Chip_ADC_StartSequencer(LPC_ADC, ADC_SEQA_IDX);
}

