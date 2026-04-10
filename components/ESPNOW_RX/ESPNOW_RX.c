#include <stdio.h>
#include "ESPNOW_RX.h"

packet_t packet_drive_rcv;  

int control_len = 6;

device_info_t controller_info  = {
    .controller_addr = {0xDC, 0xB4, 0xD9, 0x0C, 0x63, 0x70},            
    .car_addr = {0xF4, 0x65, 0x0B, 0xBB, 0x77, 0xF8}                
};



void ESPNOWconfig(){

    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    //Set wifi parameters to default
    const wifi_init_config_t default_config = WIFI_INIT_CONFIG_DEFAULT();

    //create a report variable
    esp_err_t report;

    vTaskDelay(10/ portTICK_PERIOD_MS);
    //initialize wifi
    report = esp_wifi_init(&default_config);
        if (report == ESP_OK) { printf("WIFI Init Success.\n\r"); }
        else { printf("WIFI Init Failed: %d\n\r", report); }
    //set wifi mode to station (default)
    report = esp_wifi_set_mode(WIFI_MODE_STA);
        if (report == ESP_OK) { printf("WIFI Mode Success.\n\r"); }
        else { printf("WIFI Mode Failed: %d\n\r", report); }
    //start wifi
    report = esp_wifi_start();
        if (report == ESP_OK) { printf("WIFI Start Success.\n\r"); }
        else { printf("WIFI Start Failed: %d\n\r", report); }
    //Initialize espnow
    report = esp_now_init();
        if (report == ESP_OK) { printf("ESP-NOW Init Success.\n\r"); }
        else { printf("ESP-NOW Init Failed: %d\n\r", report); }
    //setup peer information
    esp_now_peer_info_t esp_now_peer_info = {
        .peer_addr = {0xDC, 0xB4, 0xD9, 0x0C, 0x63, 0x70},
        .channel = 0,
        .ifidx = WIFI_IF_STA,
        .encrypt = 0,
    };
    esp_now_add_peer(&esp_now_peer_info);
}

//create a struct to copy the received data into

void drive_callback(const esp_now_recv_info_t *device_info, const uint8_t *data, int data_len){

    printf("Callback fired\n\r");

    //if receive from different mac address
    if((memcmp(device_info->src_addr, controller_info.controller_addr, 6) != 0)) {
        printf("Nothing received from controller / no data sent from car    \n\r");
        return;
    }

    //if receive from correct MAC but wrong lenght
    if (data_len != sizeof(packet_t)) {
        printf("Unexpected packet size: %d                                  \n\r", data_len);
        return;
    }
    //if received from correct MAC and correct lenmght
    memcpy(&packet_drive_rcv, data, sizeof(packet_t));
    printf("Signal received, Steering = %d Throtthle = %d Braking = %d  \n\r",
        packet_drive_rcv.str_dat,
        packet_drive_rcv.thrt_dat,
        packet_drive_rcv.brk_dat);

}
