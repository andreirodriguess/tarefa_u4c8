#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"

#define VRX_PIN 26  
#define VRY_PIN 27
#define RED_PIN 13  
#define BLUE_PIN 12
uint pwm_init_gpio(uint gpio, uint wrap)
{
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true);
    return slice_num;
}
float get_duty_cycle(uint16_t value)//função que calcula o duty cycle
{
    if(value > 2100)
    {

        return (value - 2047) * 2;
    }
    else if(value < 1887)
    {
        return (2047 - value) * 2;
    }
    else
    {
        return 0;
    }
}

int main()
{
    stdio_init_all();

    adc_init();
    adc_gpio_init(VRX_PIN);
    adc_gpio_init(VRY_PIN);
    uint pwm_wrap = 4096;
    uint pwm_slice_red = pwm_init_gpio(RED_PIN, pwm_wrap);
    uint pwm_slice_blue = pwm_init_gpio(BLUE_PIN, pwm_wrap);
    uint32_t last_print_time = 0;

    while (true) {
        adc_select_input(0);
        uint16_t vrx_value = adc_read();
        
        adc_select_input(1);
        uint16_t vry_value = adc_read();
        

        float duty_cycle_red = get_duty_cycle(vrx_value);
        float duty_cycle_blue = get_duty_cycle(vry_value);

        pwm_set_gpio_level(RED_PIN, duty_cycle_red);
        pwm_set_gpio_level(BLUE_PIN, duty_cycle_blue);

        uint32_t current_time = to_ms_since_boot(get_absolute_time());  

        if (current_time - last_print_time >= 1000) {  
            printf("VRX: %u\n", vrx_value); 
            printf("Duty Cycle RED LED : %.2f%%\n", duty_cycle_red/4095*100); 
            printf("VRY: %u\n", vry_value);
            printf("Duty Cycle BLUE LED: %.2f%%\n", duty_cycle_blue/4095*100);
            last_print_time = current_time;  
        }

        sleep_ms(100);  
    }
}
