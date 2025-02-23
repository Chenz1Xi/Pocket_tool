#include "board.h"
#include <stdio.h>
#include "oled.h"

//define oled
//x:0~127
//y:0~63
//x1: start ; x2: end
//y1; start ; y2: end
//mode: 0: clear ; 1: draw ; 2: fill
volatile uint32_t msg[2];
volatile unsigned int msg_flag = 1;
volatile unsigned int draw_flag = 1;
// define adc
volatile bool gCheckADC;
unsigned int adc_getValue(void);
//define smooth function
int smooth(volatile unsigned int voltage[])
{
    volatile unsigned int temp = 0;
    volatile unsigned int sum = 0;
    volatile unsigned int i = 0;
    for(i = 0; i < 5; i++)
    {
        sum = sum + voltage[i];
    }
    temp = sum / 5;
    return temp;
}

//define matrixkey
int Read_Key_Value(void)
{
	//delay_ms(10);
    volatile unsigned int key = 0;
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
uint32_t sine_calc(volatile unsigned int time, volatile unsigned int amplitude)
{
    volatile unsigned int pwmperiod = 999;
    float sine = 0;
	double pi = acos(-1);
    if(amplitude > 330){
        amplitude = 330;
    }
    else if(amplitude < 0){
        amplitude = 0;
    }
    sine = sin(pi * time / 50 - pi / 2);
    return (uint32_t)((0.5 * (1 + sine)) * pwmperiod * amplitude / 330);
}
uint32_t sq_calc(volatile unsigned int flag, volatile unsigned int amplitude)
{
    volatile unsigned int period = 999;
    //protection
    if(amplitude > 330){
        amplitude = 330;
    }
    else if(amplitude < 0){
        amplitude = 0;
    }
    if(flag == 1){
        period = period * amplitude / 330;//up
    }
    else if(flag == 0){
        period = 0;//down
    }
    return period;
}
uint32_t delta_calc(volatile unsigned int t_count, volatile unsigned int amplitude)
{
    volatile unsigned int period = 0;
    volatile unsigned int k = 20; // 1000/50 = 20
    if(amplitude > 330){
        amplitude = 330;
    }
    else if(amplitude < 0){
        amplitude = 0;
    }
    period = k * t_count * amplitude / 330 - 1;

    return period;
}
uint32_t dc_calc(volatile float amplitude)
{
    volatile unsigned int period = 999;
    if(amplitude > 330){
        amplitude = 330;
    }
    else if(amplitude < 0){
        amplitude = 0;
    }
    period = period * amplitude / 330;
    return period;
}

void get_pos(int voltage_value, volatile unsigned int max_voltage, volatile unsigned int min_voltage, volatile unsigned int *x1, volatile unsigned int *y1, volatile unsigned int *x2, volatile unsigned int *y2)
{
    volatile unsigned int v_aver = (max_voltage + min_voltage) / 2;
    volatile unsigned int voltage_span = max_voltage - min_voltage;
    *x1 = *x2;
    *y1 = *y2;
    *x2 = *x2 + 1;
    if(*x2 > 127){
        *x2 = 127;
        *x1 = 127;
    }
    if(voltage_value > v_aver)
    {
        *y2 = 23 + (voltage_value - v_aver) * 48 / voltage_span;
    }
    else
    {
        *y2 = 24 - (v_aver - voltage_value) * 48 / voltage_span;
    }
}

void deal_with_key(volatile unsigned int key_msg[], volatile unsigned int key)
{
    volatile static unsigned int pos = 1;
    if(key == 13)
    {
        pos++;
    }
    if(pos > 2)
    {
        pos = 1;
    }
    if(key <= 3)
    {
        key_msg[pos] = key;
    }
    else if(key == 5)
    {
        key_msg[pos] = 4;
    }
    else if(key == 6)
    {
        key_msg[pos] = 5;
    }
    else if(key == 7)
    {
        key_msg[pos] = 6;
    }
    else if(key == 9)
    {
        key_msg[pos] = 7;
    }
    else if(key == 10)
    {
        key_msg[pos] = 8;
    }
    else if(key == 11)
    {
        key_msg[pos] = 9;
    }
    else if(key == 14)
    {
        key_msg[pos] = 0;
    }
    else if(key == 15)
    {
        key_msg[4] = 1;
    }
}
void clear_msg(volatile unsigned int key_msg[5])
{
    volatile unsigned int i = 0;
    for(i = 0; i<5; i++)
    {
        key_msg[i] = 0;
    }
}
int main(void)
{
    board_init();
    OLED_Init();    
    OLED_Clear();

    volatile unsigned int adc_value = 0;
    volatile unsigned int voltage_value = 0;
    volatile float amplitude = 3.3;
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

    volatile int flag = 0;
    // set 
    DL_TimerG_setCaptureCompareValue(PWM_SIN_INST, 999, GPIO_PWM_SIN_C0_IDX);
    adc_value = adc_getValue();
    printf("adc value:%d\r\n", adc_value);
    max_voltage = (int)((adc_value/4095.0*3.3)*100);
    DL_TimerG_setCaptureCompareValue(PWM_SIN_INST, 0, GPIO_PWM_SIN_C0_IDX);
    adc_value = adc_getValue();
    printf("adc value:%d\r\n", adc_value);
    min_voltage = (int)((adc_value/4095.0*3.3)*100);
    msg[0] = max_voltage;
    msg[1] = min_voltage;
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
    volatile unsigned static int amplitude = 330;
    volatile unsigned static int key_count = 0;
    volatile unsigned static int old_key = 0;
    volatile unsigned static int old_key_msg[5] = {0};
    volatile unsigned static int move_allow = 0;
	volatile unsigned static int pos = 0;
    volatile unsigned int i = 0;

    volatile unsigned static int voltage_array[5] = {0};
    volatile unsigned static int array_count = 0;
    volatile unsigned int smooth_voltage = 0;

    volatile static unsigned int count_b = 0;
    volatile static unsigned int old_key_b = 0;

    volatile static unsigned int freq = 100;
    volatile static unsigned int tmax = 49;

    //get message of voltage
    if(msg_flag == 1)
    {
        max_voltage = msg[0];
        min_voltage = msg[1];
        msg_flag = 0;
        OLED_Clear();
    }

    //flag of picture
    if(x2 == 127)draw_flag = 0;
    else draw_flag = 1;

    //read key
    key = Read_Key_Value();
    printf("key = %d\n", key);
    if(key != old_key&& key == 13){
        for(i = 0; i < 5; i++){
            key_msg[i] = 0;
        }
        x1 = 0;
        x2 = 0;
        move_allow = 0;//stop drawing
        OLED_Clear();
    }
    if(key != old_key&& key == 15){
        draw_flag = 1;
    }
    //clear
    for(i = 0; i < 5; i++)
    {
        if(key_msg[i]!=old_key_msg[i])
        {
            OLED_Clear();
        }
    }
    //original set
    switch(key)
    {
        case 4:
            key_msg[0] = 4;
            break;
        case 8:
            key_msg[0] = 8;
            break;
        case 12:
            key_msg[0] = 12;
            break;
        case 16:
            key_msg[0] = 16;
        default:
            break;
    }

    //main part
    switch( DL_TimerG_getPendingInterrupt(TIMER_0_INST) )
    {
        case DL_TIMER_IIDX_ZERO:
            //printf("timer");
            if(t_count == 0) 
            {
                flag = 1;
            }
            //freq
            tmax = 5000 / freq - 1;
            if(t_count == tmax) flag = 0;
            
            //key adjust
            switch (key_msg[0])
            {
                case 0://initial
                    //oled
                    OLED_ShowString(1, 6, "A:Wave Select", 12, 1);
                    OLED_ShowString(1, 20, "B:Vpp", 12, 1);
                    OLED_ShowString(1, 36, "C:Frequency", 12, 1);
                    OLED_ShowString(1, 50, "D:Mode", 12, 1);
                    //key
                    if(key == 4){
                        
                        key_msg[0] = 4;
                        old_key = key;
                    }
                    else if(key == 8){
                        key_msg[0] = 8;
                        old_key = key;
                    }
                    else if(key == 12){
                        key_msg[0] = 12;
                        old_key = key;
                    }
                    else if(key == 16){
                        key_msg[0] = 16;
                        old_key = key;
                    }
                    break;
                case 4://select waves
                    if(key!= 0 && key!=4){
                        //OLED_Clear();
                        key_msg[1] = key;
                        old_key = key;
                    }
                    if(key_msg[1] == 0)
                    {
                        OLED_ShowString(1, 6, "1:sin", 12, 1);
                        OLED_ShowString(1, 20, "2:square", 12, 1);
                        OLED_ShowString(1, 36, "3:delta", 12, 1);
                        OLED_ShowString(1, 50, "4:dc", 12, 1);
                    }
                    else if(key_msg[1]!=0&&old_key_msg[1]==0)
                    {
                        OLED_Clear();
                    }
                    if(key_msg[1] == 1){
                        pwmperiod = sine_calc(t_count, amplitude);
                        move_allow = 1;
                    }
                    else if(key_msg[1] == 2){
                        pwmperiod = sq_calc(flag, amplitude);
                        move_allow = 1;
                    }
                    else if(key_msg[1] == 3){
                        pwmperiod = delta_calc(t_count, amplitude);
                        move_allow = 1;
                    }
                    else if(key_msg[1] == 5){
                        pwmperiod = dc_calc(amplitude);
                        move_allow = 1;
                    }
                    break;
                case 8://choose amplitude
                    //OLED_Clear();
                    
                    while(key_msg[4] != 1)
                    {
                        key = Read_Key_Value();
                        count_b++;
                        if(count_b > 5)
                        {
                            count_b = 0;
                            old_key_b = 0;
                        }
                        if(key!= old_key_b && key!=0)
                        {
                            deal_with_key(key_msg, key);
                        }
                        old_key_b = key;
                        OLED_ShowNum(50, 32, key_msg[1], 1, 12, 1);
                        OLED_ShowNum(60, 32, key_msg[2], 1, 12, 1);
                        OLED_DrawLine(55, 32, 56, 32, 1);
                        printf("key_msg = [%d,%d,%d,%d,%d]\n",key_msg[0],key_msg[1],key_msg[2],key_msg[3],key_msg[4]);
                    }
                    amplitude = key_msg[1] * 100 + key_msg[2] * 10;
                    clear_msg(key_msg);
                    break;
                case 12://choose freq
                    break;
                case 16://choose visual model
                default:
                    break;
            }
            printf("freq = %d\n", freq);
            printf("tmax = %d\n", tmax);
            printf("amplitude = %d\n", amplitude);
            printf("key_msg = [%d,%d,%d,%d,%d]\n",key_msg[0],key_msg[1],key_msg[2],key_msg[3],key_msg[4]);
            printf("old_key_msg = [%d,%d,%d,%d,%d]\n",old_key_msg[0],old_key_msg[1],old_key_msg[2],old_key_msg[3],old_key_msg[4]);
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
            //smooth signal
            if(array_count < 5){
                array_count++;
            }
            else array_count = 0;
            voltage_array[array_count] = voltage_value;
            
            smooth_voltage = smooth(voltage_array);
                            
            if(draw_flag == 1)
            {
                //draw signal
                if(move_allow == 1){
                    get_pos(smooth_voltage, max_voltage, min_voltage, &x1, &y1, &x2, &y2);
                }
                
                if(x1 < 128 && x2 < 128 &&x1 >= 3 && x2>=3&& y1 < 64 && y2<64&& y1>=0&& y2>=0){
                    if(move_allow == 1){
                        OLED_DrawLine(x1, y1, x2, y2, 1);
                        OLED_DrawLine(1, 48, 127, 48, 1);
                        OLED_ShowString(0, 55, "Vpp=", 8, 1);
                        OLED_ShowString(40, 55, "V", 8, 1);
                        OLED_ShowNum(24, 55, amplitude/100, 1, 8, 1);
                        OLED_DrawLine(30, 62, 32, 62, 1);
                        OLED_ShowNum(32, 55, amplitude/10%10, 1, 8, 1);
                    }
                }
            }
            OLED_DrawPoint(0, 63, 1);
            OLED_DrawPoint(0, 47, 1);
            OLED_DrawPoint(0, 31,1);
            OLED_DrawPoint(0, 15, 1);
            OLED_DrawPoint(0, 0, 1);
            
            OLED_Refresh();
            old_key = key;
            for(i = 0; i<5; i++)
            {
                old_key_msg[i] = key_msg[i];
            }
            break;

        default:
            break;

        NVIC_ClearPendingIRQ(TIMER_0_INST_INT_IRQN);
    }
} 