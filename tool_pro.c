#include "board.h"
#include <stdio.h>
#include "oled.h"

//define oled
//x:0~127
//y:0~63
//x1: start ; x2: end
//y1; start ; y2: end
//mode: 0: clear ; 1: draw ; 2: fill
//volatile uint32_t max = 0;
//volatile uint32_t min = 4095;
volatile uint32_t msg[2];
volatile unsigned int msg_flag = 1;
volatile unsigned int draw_flag = 1;
// define adc
volatile bool gCheckADC;
unsigned int adc_getValue(void);

//define matrixkey
int Read_Key_Value(volatile unsigned int key)
{
	//delay_ms(10);

    DL_GPIO_clearPins(GPIO_OUT_1_PORT, GPIO_OUT_1_PIN_13_PIN);
	DL_GPIO_setPins(GPIO_OUT_2_PORT, GPIO_OUT_2_PIN_16_PIN);
	DL_GPIO_setPins(GPIO_OUT_3_PORT, GPIO_OUT_3_PIN_17_PIN);
	DL_GPIO_setPins(GPIO_OUT_4_PORT, GPIO_OUT_4_PIN_28_PIN);
	if(DL_GPIO_readPins(GPIO_IN_1_PORT, GPIO_IN_1_PIN_31_PIN) == 0){
		key = 1;
	}
	else if(DL_GPIO_readPins(GPIO_IN_2_PORT, GPIO_IN_2_PIN_7_PIN) == 0){
		key = 2;
	}
	else if(DL_GPIO_readPins(GPIO_IN_3_PORT, GPIO_IN_3_PIN_8_PIN) == 0){
		key = 3;
	}
	else if(DL_GPIO_readPins(GPIO_IN_4_PORT, GPIO_IN_4_PIN_9_PIN) == 0){
		key = 4;
	}

	DL_GPIO_clearPins(GPIO_OUT_2_PORT, GPIO_OUT_2_PIN_16_PIN);
	DL_GPIO_setPins(GPIO_OUT_1_PORT, GPIO_OUT_1_PIN_13_PIN);
	DL_GPIO_setPins(GPIO_OUT_3_PORT, GPIO_OUT_3_PIN_17_PIN);
	DL_GPIO_setPins(GPIO_OUT_4_PORT, GPIO_OUT_4_PIN_28_PIN);
	if(DL_GPIO_readPins(GPIO_IN_1_PORT, GPIO_IN_1_PIN_31_PIN) == 0){
		key = 5;
	}
	else if(DL_GPIO_readPins(GPIO_IN_2_PORT, GPIO_IN_2_PIN_7_PIN) == 0){
		key = 6;
	}
	else if(DL_GPIO_readPins(GPIO_IN_3_PORT, GPIO_IN_3_PIN_8_PIN) == 0){
		key = 7;
	}
	else if(DL_GPIO_readPins(GPIO_IN_4_PORT, GPIO_IN_4_PIN_9_PIN) == 0){
		key = 8;
	}

	DL_GPIO_clearPins(GPIO_OUT_3_PORT, GPIO_OUT_3_PIN_17_PIN);
	DL_GPIO_setPins(GPIO_OUT_2_PORT, GPIO_OUT_2_PIN_16_PIN);
	DL_GPIO_setPins(GPIO_OUT_1_PORT, GPIO_OUT_1_PIN_13_PIN);
	DL_GPIO_setPins(GPIO_OUT_4_PORT, GPIO_OUT_4_PIN_28_PIN);
	if(DL_GPIO_readPins(GPIO_IN_1_PORT, GPIO_IN_1_PIN_31_PIN) == 0){
		key = 9;
	}
	else if(DL_GPIO_readPins(GPIO_IN_2_PORT, GPIO_IN_2_PIN_7_PIN) == 0){
		key = 10;
	}
	else if(DL_GPIO_readPins(GPIO_IN_3_PORT, GPIO_IN_3_PIN_8_PIN) == 0){
		key = 11;
	}
	else if(DL_GPIO_readPins(GPIO_IN_4_PORT, GPIO_IN_4_PIN_9_PIN) == 0){
		key = 12;
	}

	DL_GPIO_clearPins(GPIO_OUT_4_PORT, GPIO_OUT_4_PIN_28_PIN);
	DL_GPIO_setPins(GPIO_OUT_2_PORT, GPIO_OUT_2_PIN_16_PIN);
	DL_GPIO_setPins(GPIO_OUT_3_PORT, GPIO_OUT_3_PIN_17_PIN);
	DL_GPIO_setPins(GPIO_OUT_1_PORT, GPIO_OUT_1_PIN_13_PIN);
	if(DL_GPIO_readPins(GPIO_IN_1_PORT, GPIO_IN_1_PIN_31_PIN) == 0){
		key = 13;
	}
	else if(DL_GPIO_readPins(GPIO_IN_2_PORT, GPIO_IN_2_PIN_7_PIN) == 0){
		key = 14;
	}
	else if(DL_GPIO_readPins(GPIO_IN_3_PORT, GPIO_IN_3_PIN_8_PIN) == 0){
		key = 15;
	}
	else if(DL_GPIO_readPins(GPIO_IN_4_PORT, GPIO_IN_4_PIN_9_PIN) == 0){
		key = 16;
	}

	return key;
}

//calculate different waves
uint32_t sine_calc(int time)
{
    volatile unsigned int pwmperiod = 1000;
    float sine = 0;
	double pi = acos(-1);
    sine = sin(pi * time / 50 - pi / 2);
    return (uint32_t)((0.5 * (1 + sine)) * pwmperiod);
}
uint32_t sq_calc(volatile unsigned int flag, volatile float amplitude)
{
    volatile unsigned int period = 1000;
    //protection
    if(amplitude > 3.3){
        amplitude = 3.3;
    }
    else if(amplitude < 0){
        amplitude = 0;
    }
    if(flag == 1){
        period = period * amplitude / 3.3;//up
    }
    else{
        period = 0;//down
    }
    return period;
}
uint32_t delta_calc(volatile unsigned int t_count, volatile float amplitude)
{
    volatile unsigned int period = 0;
    volatile unsigned int k = 20; // 1000/50 = 20
    if(amplitude > 3.3){
        amplitude = 3.3;
    }
    else if(amplitude < 0){
        amplitude = 0;
    }
    period = k * t_count * amplitude / 3.3;

    return period;
}
uint32_t dc_calc(volatile float amplitude)
{
    volatile unsigned int period = 1000;
    if(amplitude > 3.3){
        amplitude = 3.3;
    }
    else if(amplitude < 0){
        amplitude = 0;
    }
    period = period * amplitude / 3.3;
    return period;
}

void get_voltage(volatile unsigned int voltage, volatile unsigned int * max_voltage, volatile unsigned int * min_voltage)
{
    if (voltage > *max_voltage)*max_voltage = voltage;
    if (voltage < *min_voltage)*min_voltage = voltage;
}

void get_pos(int voltage_value, volatile unsigned int max_voltage, volatile unsigned int min_voltage, volatile unsigned int *x1, volatile unsigned int *y1, volatile unsigned int *x2, volatile unsigned int *y2)
{
    volatile unsigned int v_aver = (max_voltage + min_voltage) / 2;
    volatile unsigned int voltage_span = max_voltage - min_voltage;
    *x1 = *x2;
    *y1 = *y2;
    *x2 = *x2 + 1.28;
    if(*x2 > 127){
        *x2 = 127;
        *x1 = 127;
    }
    printf("voltage = %d\n", voltage_value);
    //printf("v_aver = %d\n", v_aver);
    //printf("voltage_span = %d\n", voltage_span);
    if(voltage_value > v_aver)
    {
        *y2 = 31 + (voltage_value - v_aver) * 64 / voltage_span;
    }
    else
    {
        *y2 = 32 - (v_aver - voltage_value) * 64 / voltage_span;
    }
    printf("x1 = %d\n", *x1);
    printf("y1 = %d\n", *y1);
    printf("x2 = %d\n", *x2);
    printf("y2 = %d\n", *y2);
}
int main(void)
{
    board_init();
    OLED_Init();    
    OLED_Clear();

    volatile unsigned int adc_value = 0;
    volatile unsigned int voltage_value = 0;
    volatile unsigned int sine_freq = 100;
    volatile unsigned int delay_plus = 0;
    volatile unsigned int max_voltage = 0;
    volatile unsigned int min_voltage = 4095;
    delay_plus = 500 / sine_freq;
    max_voltage = 0;
    min_voltage = 4095;

    SYSCFG_DL_init();

    NVIC_ClearPendingIRQ(UART_0_INST_INT_IRQN);
    NVIC_EnableIRQ(UART_0_INST_INT_IRQN);
    NVIC_EnableIRQ(ADC_VOLTAGE_INST_INT_IRQN);

    printf("adc Demo start\r\n");

    volatile int t = 0;
    volatile int i = 0;
    volatile int flag = 0;
    // set 
    for(t = 0; t < 50; t++)
    {
        i = sine_calc(t);
        DL_TimerG_setCaptureCompareValue(PWM_SIN_INST, i, GPIO_PWM_SIN_C0_IDX);

        adc_value = adc_getValue();
        printf("adc value:%d\r\n", adc_value);

        voltage_value = (int)((adc_value/4095.0*3.3)*100);
        get_voltage(voltage_value, &max_voltage, &min_voltage);
    }
    msg[0] = max_voltage;
    msg[1] = min_voltage;
    printf("msg0=%d", msg[0]);
    printf("msg1=%d", msg[1]);
    NVIC_ClearPendingIRQ(TIMER_0_INST_INT_IRQN);
    NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);

    while (1)
    {  
        //OLED_ShowNum(1, 0, v2, 3, 1,1);
        //OLED_ShowNum(1, 15, (v_aver + v2)/2, 3, 1,1);
        //OLED_ShowNum(1, 31, v_aver, 3, 1,1);
        //OLED_ShowNum(1, 47, (v1 + v_aver)/2, 3, 1,1);
        //OLED_ShowNum(1, 63, v1, 3, 1,1);
        printf("running");
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
    volatile unsigned static int flag = 0;
    volatile static int t_count = 0;
    volatile static unsigned int pwmperiod = 0;
    volatile unsigned int adc_value = 0;
    volatile unsigned int voltage_value = 0;
    volatile unsigned static int max_voltage = 0;
    volatile unsigned static int min_voltage = 4095;
    volatile unsigned static int x1 = 0;
    volatile unsigned static int y1 = 0;
    volatile unsigned static int x2 = 0;
    volatile unsigned static int y2 = 0;
    volatile unsigned static int key = 0;
    volatile unsigned static int key_msg[5] = {0};
    volatile static float amplitude = 3;
    volatile unsigned static int key_count = 0;
    volatile unsigned static int old_key = 0;
    volatile unsigned static int move_allow = 0;
	volatile unsigned static int pos = 0;
    volatile unsigned int i = 0;

    //get message of voltage
    if(msg_flag == 1)
    {
        max_voltage = msg[0];
        min_voltage = msg[1];
        msg_flag = 0;
        OLED_Clear();
    }
    printf("max = %d\n", max_voltage);
    printf("min = %d\n", min_voltage);

    //flag of picture
    if(x2 == 127)draw_flag = 0;
    else draw_flag = 1;

    //read key
    key = Read_Key_Value(key);
    printf("key = %d\n", key);
    key_count++;
    if(key_count > 100){//delay for key
		old_key = key;
		key_count = 0;
	}
    if(key != old_key&& key == 13){
        for(i = 0; i < 5; i++){
            key_msg[i] = 0;
            x1 = 0;
            x2 = 0;
            move_allow = 0;
            OLED_Clear();
        }
    }
    if(key != old_key&& key == 15){
        draw_flag = 1;
    }

    //main part
    switch( DL_TimerG_getPendingInterrupt(TIMER_0_INST) )
    {
        case DL_TIMER_IIDX_ZERO:
            //pwm
            if(draw_flag > 0)
            {
                printf("timer");
                if(t_count == 0) 
                {
                    flag = 1;
                }
                if(t_count == 49) flag = 0;
                
                //key adjust
                switch (key_msg[0])
                {
                    case 0://initial
                        if(key == 4){
                            key_msg[0] = 4;
                            old_key = key;
                            pos++;
                        }
                        else if(key == 8){
                            key_msg[0] = 8;
                            old_key = key;
                            pos++;
                        }
                        else if(key == 12){
                            key_msg[0] = 12;
                            old_key = key;
                            pos++;
                        }
                        else if(key == 16){
                            key_msg[0] = 16;
                            old_key = key;
                            pos++;
                        }
                        break;
                    case 4://select waves
                        if(key!=old_key){
                            key_msg[1] = key;
                            old_key = key;
                        }
                        if(key_msg[1] == 1){
                            pwmperiod = sine_calc(t_count);
                            move_allow = 1;
                            pos = 0;
                        }
                        else if(key_msg[1] == 2){
                            pwmperiod = sq_calc(flag, amplitude);
                            move_allow = 1;
                            pos = 0;
                        }
                        else if(key_msg[1] == 3){
                            pwmperiod = delta_calc(t_count, amplitude);
                            move_allow = 1;
                            pos = 0;
                        }
                        else if(key_msg[1] == 5){
                            pwmperiod = dc_calc(amplitude);
                            move_allow = 1;
                            pos = 0;
                        }
                        break;
                    case 8://choose amplitude
                        break;
                    case 12://choose freq
                        break;
                    case 16://choose visual model
                    default:
                        break;
                }
                printf("key_msg = [%d,%d,%d,%d,%d]\n",key_msg[0],key_msg[1],key_msg[2],key_msg[3],key_msg[4]);
                //continue to adjust pwm
                DL_TimerG_setCaptureCompareValue(PWM_SIN_INST, pwmperiod, GPIO_PWM_SIN_C0_IDX);
                if(flag == 1) t_count++;
                if(flag == 0) t_count--;

                //adc
                adc_value = adc_getValue();
                printf("adc value:%d\r\n", adc_value);

                voltage_value = (int)((adc_value/4095.0*3.3)*100);
                printf("voltage value:%d.%d%d\r\n",
                voltage_value/100,
                voltage_value/10%10,
                voltage_value%10 );

                //draw signal
                if(move_allow == 1){
                    get_pos(voltage_value, max_voltage, min_voltage, &x1, &y1, &x2, &y2);
                }
                
                if(x1 < 128 && x2 < 128 &&x1 >= 0 && x2>=0&& y1 < 64 && y2<64&& y1>=0&& y2>=0){
                    if(move_allow == 1){
                        OLED_DrawLine(x1, y1, x2, y2, 1);
                    }
                }

            }
            
            OLED_DrawPoint(0, 63, 1);
            OLED_DrawPoint(0, 47, 1);
            OLED_DrawPoint(0, 31,1);
            OLED_DrawPoint(0, 15, 1);
            OLED_DrawPoint(0, 0, 1);
            OLED_Refresh();

            break;

        default:
            break;

        NVIC_ClearPendingIRQ(TIMER_0_INST_INT_IRQN);
    }
} 