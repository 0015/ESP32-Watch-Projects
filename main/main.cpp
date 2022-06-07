#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "sdkconfig.h"

// ADC
#include <driver/adc.h>
#include <esp_adc_cal.h>
#define CONV_FACTOR 1.98
#define DEFAULT_VREF 1100
#define NUM_SAMPLES 64

// SNTP
#include <time.h>
#include <sys/time.h>
#include "esp_system.h"
#include "protocol_examples_common.h"
#include "esp_sntp.h"

#include <button.h>
#include "app_display.h"
#include "openhaystack.h"

// todo: Set your TimeZone
const char *timezone = "PST8PDT";
uint16_t view_num = 0;
bool openHaystackOn = false;

static const char *TAG = "ESP32-Watch";
extern "C"
{
    void app_main(void);
}

void battery_task(void *pvParameter)
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);
    esp_adc_cal_characteristics_t *adc_chars = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);

    while (1)
    {
        uint32_t adc_reading = 0;
        for (int i = 0; i < NUM_SAMPLES; i++)
        {
            adc_reading += adc1_get_raw((adc1_channel_t)ADC1_CHANNEL_6);
        }
        adc_reading /= NUM_SAMPLES;

        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
        float batVol = voltage * CONV_FACTOR / 1000;
        show_battery_icon(batVol);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void buttons_task(void *pvParameter)
{

    button_event_t ev;
    QueueHandle_t button_events = button_init(PIN_BIT(GPIO_NUM_26) | PIN_BIT(GPIO_NUM_27));

    while (true)
    {
        if (xQueueReceive(button_events, &ev, 1000 / portTICK_PERIOD_MS))
        {
            if ((ev.pin == GPIO_NUM_26) && (ev.event == BUTTON_DOWN))
            {
                if (view_num == 1){
                    if(!openHaystackOn){
                        openhaystack_on();
                    }else
                    {
                        openhaystack_off();
                    }
                    openHaystackOn = !openHaystackOn;
                    show_tracking(openHaystackOn);
                }
                        }
            else if ((ev.pin == GPIO_NUM_27) && (ev.event == BUTTON_DOWN))
            {
                view_num++;
                if (view_num > 1)
                    view_num = 0;
                screen_page(view_num);
            }
        }
    }
}

//SNTP
static void obtain_time(void);
static void initialize_sntp(void);
void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

static void obtain_time(void)
{
    ESP_ERROR_CHECK(example_connect());

    initialize_sntp();

    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count)
    {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    time(&now);
    localtime_r(&now, &timeinfo);

    ESP_ERROR_CHECK(example_disconnect());
}

static void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_init();
}

void time_task(void *pvParameter)
{
    time_t now;
    struct tm timeinfo;
    //char strftime_buf[64];
    char hour_buf[3];
    char minute_buf[3];
    char second_buf[3];
    char month_date_buf[7];
    char weekday_buf[4];

    obtain_time();
    //setenv("TZ", "PST8PDT,M3.2.0,M11.1.0", 1);
    setenv("TZ", timezone, 1);
    tzset();
    loading_spinner_delete();
    while (true)
    {
        time(&now);
        localtime_r(&now, &timeinfo);
        strftime(hour_buf, 3, "%H", &timeinfo);
        strftime(minute_buf, 3, "%M", &timeinfo);
        strftime(second_buf, 3, "%S", &timeinfo);
        strftime(month_date_buf, 7, "%b %d", &timeinfo);
        strftime(weekday_buf, 4, "%a", &timeinfo);
        show_date_time(hour_buf, minute_buf, second_buf, month_date_buf, weekday_buf);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    bt_controller_init();
    
    lvgl_init();
    lv_draw_ui();

    xTaskCreate(&buttons_task, "buttons_task", CONFIG_ESP32_BUTTON_TASK_STACK_SIZE, NULL, 5, NULL);
    xTaskCreate(&battery_task, "battery_task", 2048, NULL, 5, NULL);
    xTaskCreate(&time_task, "time_task", 4096, NULL, 5, NULL);

    while (1)
    {
        lv_timer_handler();
        vTaskDelay(1);
    }
}