#ifndef _OPENHAYSTACK_H_
#define _OPENHAYSTACK_H_

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "nvs_flash.h"
#include "esp_partition.h"

#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_defs.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void bt_controller_init(void);
    void openhaystack_on(void);
    void openhaystack_off(void);

#if __cplusplus
}
#endif

#endif
