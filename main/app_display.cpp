#include "app_display.h"

// todo: Set your display 
static const uint16_t screenWidth = 240;
static const uint16_t screenHeight = 240;

class LGFX : public lgfx::LGFX_Device
{
    lgfx::Panel_GC9A01 _panel_instance;
    lgfx::Bus_SPI _bus_instance;
    lgfx::Light_PWM _light_instance;

public:
    LGFX(void)
    {
        {                                      
            auto cfg = _bus_instance.config();
            cfg.spi_host = VSPI_HOST;  
            cfg.spi_mode = 0;          
            cfg.freq_write = 40000000; 
            cfg.freq_read = 16000000;  
            cfg.spi_3wire = false;     
            cfg.use_lock = true;       
            cfg.dma_channel = 1;       
            cfg.pin_sclk = 18;         
            cfg.pin_mosi = 23;         
            cfg.pin_miso = -1;         
            cfg.pin_dc = 14;           

            _bus_instance.config(cfg);  
            _panel_instance.setBus(&_bus_instance);
        }

        {                                       
            auto cfg = _panel_instance.config();

            cfg.pin_cs = 4;
            cfg.pin_rst = 12;
            cfg.pin_busy = -1;
            cfg.memory_width = screenWidth;
            cfg.memory_height = screenHeight;
            cfg.panel_width = screenWidth;
            cfg.panel_height = screenHeight;
            cfg.offset_x = 0;      
            cfg.offset_y = 0;     
            cfg.offset_rotation = 0;
            cfg.dummy_read_pixel = 8;
            cfg.dummy_read_bits = 1;
            cfg.readable = true;
            cfg.invert = true;
            cfg.rgb_order = false;
            cfg.dlen_16bit = false;
            cfg.bus_shared = true;

            _panel_instance.config(cfg);
        }

        {
            auto cfg = _light_instance.config();
            cfg.pin_bl = 33;
            cfg.invert = false;
            cfg.freq = 44100;
            cfg.pwm_channel = 7;

            _light_instance.config(cfg);
            _panel_instance.setLight(&_light_instance);
        }

        setPanel(&_panel_instance);
    }
};
static LGFX lcd;

#define LV_TICK_PERIOD_MS 1
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10];
static lv_style_t style_black;

uint16_t chartSize = screenWidth - 20;
uint16_t center_x = screenWidth / 2;
uint16_t center_y = screenWidth / 2;
uint16_t radius = 80;
uint16_t angle = 0;
float degree = 3.14159 / 180;
bool isRunningTracking = false;

lv_obj_t *tabview;
lv_obj_t *loading_spinner;
lv_obj_t *battery_label;
lv_obj_t *clock_hour_label;
lv_obj_t *clock_minute_label;
lv_obj_t *clock_second_label;
lv_obj_t *month_date_label;
lv_obj_t *weekday_label;
lv_obj_t *chart;
lv_obj_t *tracking_label;

static void
lv_tick_task(void *arg)
{
    (void)arg;
    lv_tick_inc(LV_TICK_PERIOD_MS);
}

void display_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    lcd.startWrite();
    lcd.setAddrWindow(area->x1, area->y1, w, h);
    lcd.pushPixels((uint16_t *)&color_p->full, w * h, true);
    lcd.endWrite();

    lv_disp_flush_ready(disp);
}

void lvgl_init()
{
    lcd.init(); // Initialize LovyanGFX
    // lcd.setBrightness(128); // back-light
    lv_init(); // Initialize lvgl
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 10);
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = display_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"};
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));
}

void show_battery_icon(float voltage){
    if (voltage >= 4.1)
    {
        lv_label_set_text(battery_label, LV_SYMBOL_BATTERY_FULL "");
    }
    else if (voltage < 4.1 && voltage >= 4.0)
    {
        lv_label_set_text(battery_label, LV_SYMBOL_BATTERY_3 "");
    }
    else if (voltage < 4.0 && voltage >= 3.9)
    {
        lv_label_set_text(battery_label, LV_SYMBOL_BATTERY_2 "");
    }
    else if (voltage < 3.9 && voltage >= 3.8)
    {
        lv_label_set_text(battery_label, LV_SYMBOL_BATTERY_1 "");
    }
    else
    {
        lv_label_set_text(battery_label, LV_SYMBOL_BATTERY_EMPTY "");
    }
}

void screen_page(uint16_t page_num){
    lv_tabview_set_act(tabview, page_num, LV_ANIM_ON);
}

void show_date_time(char *hour, char *minute, char *second, char *month_date, char *weekday){
    lv_label_set_text(clock_hour_label, hour);
    lv_label_set_text(clock_minute_label, minute);
    lv_label_set_text(clock_second_label, second);
    lv_label_set_text(month_date_label, month_date);
    lv_label_set_text(weekday_label, weekday);
}

void show_tracking(bool isOn){
    isRunningTracking = isOn;
    lv_label_set_text(tracking_label, isOn ? LV_SYMBOL_GPS "#ff0000  Being Tracked! (Powered by openHaystack)" : LV_SYMBOL_CLOSE "#ffff00  Tracking Stopped!");
}

void drawCircle()
{
    if (angle >= 360)
    {
        angle = 0;
    }

    int x = center_x + radius * cos(-angle * degree);
    int y = center_y + radius * sin(-angle * degree);

    lv_chart_set_next_value2(chart, lv_chart_get_series_next(chart, NULL), lv_rand(x - 10, x + 10), lv_rand(y - 10, y + 10));
    angle += 5;
}

void drawRandom()
{
    lv_chart_set_next_value2(chart, lv_chart_get_series_next(chart, NULL), lv_rand(0, chartSize), lv_rand(0, chartSize));
}

static void add_data(lv_timer_t *timer)
{
    LV_UNUSED(timer);
    if (isRunningTracking)
        drawCircle();
    else
        drawRandom();
}

static void draw_event_cb(lv_event_t *e)
{
    lv_obj_draw_part_dsc_t *dsc = lv_event_get_draw_part_dsc(e);
    if (dsc->part == LV_PART_ITEMS)
    {
        lv_obj_t *obj = lv_event_get_target(e);
        lv_chart_series_t *ser = lv_chart_get_series_next(obj, NULL);
        uint32_t cnt = lv_chart_get_point_count(obj);
        dsc->rect_dsc->bg_opa = (LV_OPA_COVER * dsc->id) / (cnt - 1);

        lv_coord_t *x_array = lv_chart_get_x_array(obj, ser);
        lv_coord_t *y_array = lv_chart_get_y_array(obj, ser);

        uint32_t start_point = lv_chart_get_x_start_point(obj, ser);
        uint32_t p_act = (start_point + dsc->id) % cnt;
        lv_opa_t x_opa = (x_array[p_act] * LV_OPA_50) / 320;
        lv_opa_t y_opa = (y_array[p_act] * LV_OPA_50) / 320;

        dsc->rect_dsc->bg_color = lv_color_mix(lv_color_hex(0xFFFFFF),
                                               lv_palette_main(LV_PALETTE_GREY),
                                               x_opa + y_opa);
    }
}

void loading_spinner_make(char *title)
{
    loading_spinner = lv_spinner_create(tabview, 1000, 60);
    lv_obj_set_size(loading_spinner, 240, 240);
    lv_obj_center(loading_spinner);

    lv_obj_t *loading_label = lv_label_create(loading_spinner);
    lv_label_set_text(loading_label, title);
    lv_obj_center(loading_label);
}

void loading_spinner_delete()
{
    lv_obj_del(loading_spinner);
}

void lv_draw_ui(void)
{

    lv_style_init(&style_black);
    lv_style_set_border_width(&style_black, 0);
    lv_style_set_radius(&style_black, 0);
    lv_style_set_bg_color(&style_black, lv_color_hex(0x000000));

    tabview = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 0);
    lv_obj_add_style(tabview, &style_black, 0);

    /*Add 3 tabs (the tabs are page (lv_page) and can be scrolled*/
    lv_obj_t *tab1 = lv_tabview_add_tab(tabview, "Clock");
    lv_obj_t *tab2 = lv_tabview_add_tab(tabview, "Tag");

    battery_label = lv_label_create(tab1);
    lv_label_set_text(battery_label, LV_SYMBOL_BATTERY_FULL "");
    lv_obj_align(battery_label, LV_ALIGN_TOP_MID, 0, 0);

    clock_hour_label = lv_label_create(tab1);
    lv_obj_align(clock_hour_label, LV_ALIGN_CENTER, 0, -20);
    lv_label_set_text(clock_hour_label, "12");
    lv_obj_set_style_text_font(clock_hour_label, &lv_font_montserrat_48, LV_PART_MAIN | LV_STATE_DEFAULT);

    clock_minute_label = lv_label_create(tab1);
    lv_obj_align(clock_minute_label, LV_ALIGN_CENTER, 0, 20);
    lv_label_set_text(clock_minute_label, "30");
    lv_obj_set_style_text_font(clock_minute_label, &lv_font_montserrat_48, LV_PART_MAIN | LV_STATE_DEFAULT);

    clock_second_label = lv_label_create(tab1);
    lv_obj_align(clock_second_label, LV_ALIGN_CENTER, 60, 0);
    lv_label_set_text(clock_second_label, "30");
    lv_obj_set_style_text_font(clock_second_label, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);

    weekday_label = lv_label_create(tab1);
    lv_obj_align(weekday_label, LV_ALIGN_LEFT_MID, 0, -10);
    lv_label_set_text(weekday_label, "");
    lv_obj_set_style_text_font(weekday_label, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);

    month_date_label = lv_label_create(tab1);
    lv_obj_align(month_date_label, LV_ALIGN_LEFT_MID, 0, 10);
    lv_label_set_text(month_date_label, "");
    lv_obj_set_style_text_font(month_date_label, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);

    chart = lv_chart_create(tab2);
    lv_obj_add_style(chart, &style_black, 0);
    lv_obj_set_size(chart, chartSize, chartSize);
    lv_obj_center(chart);
    lv_obj_add_event_cb(chart, draw_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);
    lv_obj_set_style_line_width(chart, 0, LV_PART_ITEMS);

    lv_chart_set_type(chart, LV_CHART_TYPE_SCATTER);
    lv_chart_set_div_line_count(chart, 0, 0);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_X, 0, chartSize);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, chartSize);
    lv_chart_set_point_count(chart, 50);

    lv_chart_series_t *ser = lv_chart_add_series(chart, lv_color_white(), LV_CHART_AXIS_PRIMARY_Y);
    for (int i = 0; i < 50; i++)
    {
        lv_chart_set_next_value2(chart, ser, lv_rand(0, chartSize), lv_rand(0, chartSize));
    }

    lv_timer_create(add_data, 1, chart);

    tracking_label = lv_label_create(tab2);
    lv_label_set_recolor(tracking_label, true);
    lv_label_set_text(tracking_label, LV_SYMBOL_CLOSE "#ffff00  Tracking Stopped!");
    lv_label_set_long_mode(tracking_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_width(tracking_label, 150);
    lv_obj_align(tracking_label, LV_ALIGN_CENTER, 0, 0);

    lv_obj_clear_flag(tab2, LV_OBJ_FLAG_SCROLLABLE);
    lv_tabview_set_act(tabview, 0, LV_ANIM_ON);

    loading_spinner_make("System Loading...");
}