#include "board.h"
#include <stdio.h>
#include "oled.h"

//define oled
//x:0~127
//y:0~63
//x1: start ; x2: end
//y1; start ; y2: end
//mode: 0: clear ; 1: draw ; 2: fill 
//volatile unsigned int x1 = 0, y1 = 0, x2 = 0, y2 = 0, mode = 1;
//volatile int voltage_span = 0;

// define pwm
volatile unsigned int pwmperiod = 1000;
volatile float pwm_freq = 4000.0;
volatile int sine_freq = 100;

volatile unsigned int delay_plus = 0;// original delay 1us * plus = final delay
//delay_plus = 500 / sine_freq;
// define adc
volatile unsigned int delay_times = 0;
volatile bool gCheckADC;        //flag to check ADC value

unsigned int adc_getValue(void);//read ADC value
//have defined delay_us(unsigned long us)

//calculate sine wave
uint32_t sine_calc(int time)
{
    float sine = 0;
	double pi = acos(-1);
    sine = sin(3 * pi * time / 1000 - pi / 2);
    return (uint32_t)((0.5 * (1 + sine)) * pwmperiod);
}

void get_voltage(volatile unsigned int voltage, volatile unsigned int * max_voltage, volatile unsigned int * min_voltage, volatile unsigned int * voltage_span,volatile unsigned  int * v_aver)
{
    int i = 0;
    int t = 0;
    if (voltage > *max_voltage)*max_voltage = voltage;
    if (voltage < *min_voltage)*min_voltage = voltage;
    *voltage_span = *max_voltage - *min_voltage;
    *v_aver = (*max_voltage + *min_voltage) / 2;
}

void get_pos(volatile unsigned int *x1, volatile unsigned int *y1, volatile unsigned int *x2, volatile unsigned int *y2, int t, int flag, int voltage_value, int v_aver, int voltage_span)
{
    *x1 = *x2;
    *y1 = *y2;
    *x2 = *x2 + 5;
    if(*x2 > 127){
        *x2 = *x2 - 128;
        *x1 = 0;
    }
    //if(flag == 1)*x2 = t * 128 / 1000;
    //if(flag == 0)*x2 = 127 - t * 128 / 1000;
    *y2 = 31 + (voltage_value - v_aver) * 64 / voltage_span;
}
int main(void)
{
    board_init();
    OLED_Init();    
    OLED_Clear();
    //OLED_DisPlay_On();
    //****************************	
    //set delay_plus at here
    delay_plus = (int)(500 / sine_freq);
		
    //scanf("%d", &period);
    //delay_plus = period;
    //****************************

    volatile unsigned int x1 = 0, y1 = 0, x2 = 0, y2 = 0;
    unsigned int adc_value = 0;
    unsigned int voltage_value = 0;
    volatile unsigned int voltage_span = 0;
    volatile unsigned int v_aver = 0;
    volatile unsigned int max_voltage = 0;
    volatile unsigned int min_voltage = 0;
    max_voltage = 0;
    min_voltage = 4095;

    SYSCFG_DL_init();

    NVIC_ClearPendingIRQ(UART_0_INST_INT_IRQN);
    NVIC_EnableIRQ(UART_0_INST_INT_IRQN);
    NVIC_EnableIRQ(ADC_VOLTAGE_INST_INT_IRQN);
    NVIC_ClearPendingIRQ(TIMER_0_INST_INT_IRQN);
    NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);

    printf("adc Demo start\r\n");

    volatile int t = 0;
    volatile int i = 0;
    volatile int flag = 0;
    x1 = 1;
    y1 = 31;
    x2 = 0;
    y2 = 31;

    // set 
    for(t = 0; t < 1000; t++)
    {
        i = sine_calc(t);
        DL_TimerG_setCaptureCompareValue(PWM_SIN_INST, i, GPIO_PWM_SIN_C0_IDX);
        delay_us(delay_plus);

        adc_value = adc_getValue();
        printf("adc value:%d\r\n", adc_value);

        voltage_value = (int)((adc_value/4095.0*3.3)*100);
        get_voltage(voltage_value, &max_voltage, &min_voltage, &voltage_span, &v_aver);
        delay_us(delay_plus);
    }
    t = 0;

    while (1)
    {
        // pwm part
        if(t == 0) flag = 1;
        if(t == 1000) flag = 0;
        i = sine_calc(t);
        DL_TimerG_setCaptureCompareValue(PWM_SIN_INST, i, GPIO_PWM_SIN_C0_IDX);
        if(flag == 1) t++;
        if(flag == 0) t--;
        
			
        // adc part
        adc_value = adc_getValue();
        printf("adc value:%d\r\n", adc_value);

        voltage_value = (int)((adc_value/4095.0*3.3)*100);

        printf("voltage value:%d.%d%d\r\n",
        voltage_value/100,
        voltage_value/10%10,
        voltage_value%10 );

        //printf("max_voltage = %d\n", max_voltage);
        //printf("min_voltage = %d\n", min_voltage);

        // oled part     
        //printf("voltage_span = %d\n", voltage_span);
        if(voltage_span > 100)
        {
            //
        }
        else
        {
            printf("pass\n");
        }
				
        if(t % 80 == 0)
        {
            get_pos(&x1, &y1, &x2, &y2, t, flag, voltage_value, v_aver, voltage_span);
            printf("x1 = %d\n", x1);
            printf("y1 = %d\n", y1);
            printf("x2 = %d\n", x2);
            printf("y2 = %d\n", y2);
            if(x1 < 128 && x2 < 128 &&x1 >= 0 && x2>=0&& y1 < 64 && y2<64&& y1>=0&& y2>=0){
                OLED_DrawLine(x1, y1, x2, y2, 1);
            }
        }
        OLED_DrawPoint(0, 63, 1);
        OLED_DrawPoint(0 , 47, 1);
        OLED_DrawPoint(0, 31,1);
        OLED_DrawPoint(0, 15, 1);
        OLED_DrawPoint(0, 0, 1);
        //OLED_ShowNum(1, 0, v2, 3, 1,1);
        //OLED_ShowNum(1, 15, (v_aver + v2)/2, 3, 1,1);
        //OLED_ShowNum(1, 31, v_aver, 3, 1,1);
        //OLED_ShowNum(1, 47, (v1 + v_aver)/2, 3, 1,1);
        //OLED_ShowNum(1, 63, v1, 3, 1,1);
        OLED_Refresh();
        delay_us(delay_plus);
    }    
    
}

//read ADC value
unsigned int adc_getValue(void)
{
        unsigned int gAdcResult = 0;

        //start ADC conversion
        DL_ADC12_startConversion(ADC_VOLTAGE_INST);
        //if ADC conversion is not finished, wait for it
        while (false == gCheckADC) {
            __WFE();
        }
        //get ADC result
        gAdcResult = DL_ADC12_getMemResult(ADC_VOLTAGE_INST, ADC_VOLTAGE_ADCMEM_ADC_CH0);

        //clear ADC conversion flag
        gCheckADC = false;

        return gAdcResult;
}

//ADC interrupt handler
void ADC_VOLTAGE_INST_IRQHandler(void)
{
        //check interrupt source
        switch (DL_ADC12_getPendingInterrupt(ADC_VOLTAGE_INST))
        {
                        //check whether ADC conversion is finished
                        case DL_ADC12_IIDX_MEM0_RESULT_LOADED:
                                        gCheckADC = true;//set sginal to 1
                                        break;
                        default:
                                        break;
        }
}

// timer interrupt
void TIMER_0_INST_IRQHandler(void)
{
    switch( DL_TimerG_getPendingInterrupt(TIMER_0_INST) )
    {
        case DL_TIMER_IIDX_ZERO:
            //
            break;

        default:
            break;
    }
}