#ifndef _APP_DISPLAY_H_
#define _APP_DISPLAY_H_

#define LGFX_USE_V1
#include <LovyanGFX.h>
#include <lvgl.h>
#if __cplusplus
extern "C"
{
#endif

    void lvgl_init();
    void lv_draw_ui();
    void loading_spinner_make(char *title);
    void loading_spinner_delete();
    void show_battery_icon(float voltage);
    void screen_page(uint16_t page_num);
    void show_date_time(char *hour, char *minute, char *second, char *month_date, char *weekday);
    void show_tracking(bool isOn);

#if __cplusplus
}
#endif
#endif /*_APP_DISPLAY_H_*/