#include <stdio.h>
#include <string.h>

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"

#include "radio.h"
#include "ESPNOW_RX.h"


#define servo 0
#define motor 2
#define IN1 16
#define IN2 17

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

        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, packet_drive_rcv.str_dat);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);

        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, packet_drive_rcv.thrt_dat);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

//background task (setting values, radio transmits)
void RX_Print_Background(void *pvParameters){

    printf("Controls receive program\n");

    // verify SPI comms
    uint8_t ch = nrf_read_reg(NRF_RF_CH);
    uint8_t cfg = nrf_read_reg(NRF_CONFIG);
    printf("RF channel: 0x%02X (expect 0x7C)\n", ch);
    printf("CONFIG: 0x%02X (expect 0x0B)\n", cfg);

    vTaskDelay(200 / portTICK_PERIOD_MS);

    //hides the cursor
    printf("\033[?25l");

    printf("CONFIG:    0x%02X (expect 0x0F for RX)\n",  nrf_read_reg(0x00));
    printf("EN_AA:     0x%02X (expect 0x00)\n",          nrf_read_reg(0x01));
    printf("EN_RXADDR: 0x%02X (expect 0x01)\n",          nrf_read_reg(0x02));
    printf("AW:        0x%02X (expect 0x03)\n",          nrf_read_reg(0x03));
    printf("RETR:      0x%02X (expect 0x00)\n",          nrf_read_reg(0x04));
    printf("RF_CH:     0x%02X (expect 0x7C)\n",          nrf_read_reg(0x05));
    printf("RF_SETUP:  0x%02X (expect 0x27)\n",          nrf_read_reg(0x06));
    printf("STATUS:    0x%02X (expect 0x0E)\n",          nrf_read_reg(0x07));
    printf("RX_PW_P0:  0x%02X (expect 0x04)\n",          nrf_read_reg(0x11));

    while(1){

        if(nrf_data_available()){

            nrf_receive(data, 4);

            //Reconstruct uint16_t values for steering and t hrottle duty cycle
            duty_steering = (uint16_t)(((data[0] & 0x0F) << 8) | data[1]);
            duty_throttle = (uint16_t)(((data[2] & 0x0F) << 8)| data[3]);

            //Print received values
            printf("RX success | Steering Duty Cycle: %4d , Throttle Duty Cycle: %4d\r",
                duty_steering, duty_throttle);

            nrf_write_reg(NRF_STATUS, 0x01);

        }

        else{

            uint8_t status = nrf_read_reg(NRF_STATUS);

            if(status & 0x40){

                nrf_write_reg(NRF_STATUS, 0x70);

            }
            printf("RX failed | No data to display. 0x%02X                                \r", status);
        }

        vTaskDelay( 100 / portTICK_PERIOD_MS);

    }

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
        .freq_hz = 5000,                         // 5 kHz for servos
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

