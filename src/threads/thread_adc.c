#include "thread_adc.h"
#include "thread_uart.h"
#include "adc.h"
#include <stdio.h>
#include "pools.h"
#include "AdcLib.h"
#include "timer.h"

//#define GET_ECG_CONTINUOUS
#define GET_HS_CONTINUOUS

osThreadId tid_Thread_adc;

#define MAX_ECG_ADC_VALUE 0x10000000
#define ECG_ADC_TO_VOLTAGE 1.2
#define MAX_HS_ADC_VALUE 0x10000000
#define HS_ADC_TO_VOLTAGE 2.8

#define Q_ADCVALUE_SIZE 20
osMessageQDef (Q_ADCVALUE, Q_ADCVALUE_SIZE, uint32_t);
osMessageQId Q_ADCVALUE;
_PoolDef(P_ADCVALUE, Q_ADCVALUE_SIZE, AdcValueDef);

static enum AdcDataType current_adc_type;

EcgDataDef ecg_frame = {0,0,0};
// sampling ECG signal
void Thread_adc (void const *argument)
{
	int32_t tickcount_start;
	AdcValueDef *pdata;
	osEvent os_result;

	ADC1_init();
	ecg_frame.ecg_data = 0;
	ecg_frame.hs_data = 0;
	
	// Create message queue.
	Q_ADCVALUE = osMessageCreate(osMessageQ(Q_ADCVALUE), NULL);

	#if defined(GET_ECG_CONTINUOUS)
		ECG_start_continuous();
		current_adc_type = ecg;
	#elif defined(GET_HS_CONTINUOUS)
		HS_start_continuous();
		current_adc_type = hs;
	#endif

	// store starting tick count.
	tickcount_start = getCurrentCount_Timer1();
	while(1) {
		os_result = osMessageGet(Q_ADCVALUE, osWaitForever);
		if(os_result.status == osEventMessage){
			pdata = (AdcValueDef*)os_result.value.v;
			ecg_frame.date = (pdata->date - tickcount_start);
			if(pdata->type == ecg){
				//ecg_frame.ecg_data = ECG_ADC_TO_VOLTAGE * pdata->adc / MAX_ECG_ADC_VALUE;
				ecg_frame.ecg_data = pdata->adc;
			} else if (pdata->type == hs){
				//ecg_frame.hs_data = HS_ADC_TO_VOLTAGE * pdata->adc / MAX_HS_ADC_VALUE;
				ecg_frame.hs_data = pdata->adc;
			}
			UART_Write_Frame(0x01, sizeof(EcgDataDef), &ecg_frame);
		}
	}
}

void ADC1_Int_Handler(void)
{
	volatile int f_ADCSTA = 0;
	AdcValueDef *pdata;
	int32_t data;
	f_ADCSTA = AdcSta(pADI_ADC1);	// Read ADC status register to clear
	if(f_ADCSTA & DETSTA_STEPDATRDY){
		data = AdcRd(pADI_ADC1);            // Read ADC result register
		pdata = _PoolAlloc(P_ADCVALUE);
		pdata->adc = data;
		pdata->date = getCurrentCount_Timer1();
		pdata->type = current_adc_type;
		osMessagePut(Q_ADCVALUE, (uint32_t)pdata, 0);
	}
}
