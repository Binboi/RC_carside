#include <stdio.h>
#include <string.h>

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"

#include "radio.h"
#include "ESPNOW_RX.h"


#define servo 16
#define motor 17


uint16_t duty_steering;
uint16_t duty_throttle;
uint16_t duty_brake;
uint8_t data[PAYLOAD_SIZE];

void pwm_config();
void RX_Print_Background(void *pvParameters);
void setVal_Background(void *pvParameters);

void app_main(void){

    //Initialize pwm channels
    pwm_config();

    //Initialize radio protocol
    //nrf_init();
    //nrf_flush_rx();
    //nrf_flush_tx();

    //Start receving
    //nrf_start_rx();
    //vTaskDelay(20 / portTICK_PERIOD_MS);

    //Start the background car value setting process
    //xTaskCreate(RX_Print_Background, "Live Values", 2048, NULL, 5, NULL);

    //Start the background printing process
    //xTaskCreate(setVal_Background, "Live Values", 2048, NULL, 5, NULL);
   
    ESPNOWconfig();
    esp_now_register_recv_cb(drive_callback);

    while(1){

        //ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, packet_drive_rcv.str_dat);
        //ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);

        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, packet_drive_rcv.thrt_dat);
        //ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 370);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);

            printf("Throttle: %4d \r", packet_drive_rcv.thrt_dat);

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

//background task (setting values, radio transmits)
void RX_Print_Background(void *pvParameters){

    

}

    void pwm_config(){

    // Setup the timer for the servos (slower)
    ledc_timer_config_t servo_timer_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_12_BIT,  // 12-bit = 0-4095
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 50,                         // 50 Hz for servos
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&servo_timer_config);

    // Setup the timer for the motor (faster)
    ledc_timer_config_t motor_timer_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_12_BIT,  // 12-bit = 0-4095
        .timer_num = LEDC_TIMER_1,
        .freq_hz = 50,                         // 50Hz for ESC (motor controller)
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&motor_timer_config);

    // Setup the PWM channel for the steering
    ledc_channel_config_t channel_config = {
        .gpio_num = servo,
        .speed_mode = LEDC_LOW_SPEED_MODE,  
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&channel_config);

    // Setup the PWM channel for EN1 (on driving motor)
    ledc_channel_config_t channel_config2 = {
        .gpio_num = motor,
        .speed_mode = LEDC_LOW_SPEED_MODE,

        .channel = LEDC_CHANNEL_1,
        .timer_sel = LEDC_TIMER_1,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&channel_config2);

}

