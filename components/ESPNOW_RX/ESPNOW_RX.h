#pragma once

#include <stdio.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_now.h"
#include "nvs_flash.h"

extern int control_len;

void ESPNOWconfig();

typedef struct __attribute__((packed)) {
    uint16_t str_dat;
    uint16_t thrt_dat;
    uint16_t brk_dat;
} packet_t;

extern packet_t packet_drive_rcv;

typedef struct controller_info{
    uint8_t controller_addr[6];            
    uint8_t car_addr[6];            
} device_info_t;


//void ESPNOWconfig();

//callback function of driving data packets
void drive_callback(const esp_now_recv_info_t *device_info, const uint8_t *data, int data_len);

 //const typedef struct esp_now_info {
   
 //} esp_now_recv_info_t;

 //const uint8_t *data;
 //int data_len;