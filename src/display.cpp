#include <TFT_eSPI.h>
#include <lvgl.h>
#include "lv_driver.h"
#include "pinconfig.h"

TFT_eSPI screen = TFT_eSPI();
static lv_obj_t* status_label = nullptr;
static lv_obj_t* log_box = nullptr;
static String log_lines[1000];
static uint16_t log_index = 0;

void writeLog(const char* line) {
    Serial.println(line);

    if (log_index >= 1000) {
        // Shift lines up
        for (int i = 1; i < 1000; ++i) {
            log_lines[i - 1] = log_lines[i];
        }
        log_index = 999;
    }

    log_lines[log_index++] = line;

    // Rebuild text
    String content;
    for (int i = 0; i < log_index; ++i) {
        content += log_lines[i];
        content += "\n";
    }

    lv_textarea_set_text(log_box, content.c_str());
    lv_textarea_set_cursor_pos(log_box, LV_TEXTAREA_CURSOR_LAST);
}

void initDisplay() {
    screen.init();
    screen.setRotation(1);
    screen.fillScreen(TFT_BLACK);
    pinMode(TFT_LEDA_PIN, OUTPUT);
    digitalWrite(TFT_LEDA_PIN, 0);

    screen.setTextFont(1);
    screen.setTextColor(TFT_GREEN, TFT_BLACK);
    delay(1000);

    Serial.begin(115200);

    lvgl_init();

    // Apply dark theme with small font
    lv_theme_t* dark = lv_theme_default_init(nullptr,
                                             lv_palette_main(LV_PALETTE_BLUE),
                                             lv_palette_main(LV_PALETTE_GREY),
                                             true,
                                             &lv_font_montserrat_10);
    lv_disp_set_theme(nullptr, dark);

    // Create status bar
    lv_obj_t* status_bar = lv_obj_create(lv_scr_act());
    lv_obj_set_size(status_bar, LV_HOR_RES, 20);
    lv_obj_set_style_bg_color(status_bar, lv_color_make(255, 140, 0), 0); // retro orange
    lv_obj_set_style_border_width(status_bar, 0, 0);
    lv_obj_clear_flag(status_bar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(status_bar, LV_ALIGN_TOP_MID, 0, 0);

    // Status label
    status_label = lv_label_create(status_bar);
    lv_label_set_text(status_label, "geogram uptime: 00:00:00");
    lv_obj_set_style_text_font(status_label, &lv_font_montserrat_10, LV_PART_MAIN);
    lv_obj_set_style_text_color(status_label, lv_color_black(), LV_PART_MAIN);
    lv_obj_align(status_label, LV_ALIGN_LEFT_MID, 4, 0);

    // Create log text box
    log_box = lv_textarea_create(lv_scr_act());
    lv_obj_set_size(log_box, LV_HOR_RES, LV_VER_RES - 20);  // Full width, below status bar
    lv_obj_align(log_box, LV_ALIGN_TOP_LEFT, 0, 20);         // y = 20 to sit below status bar
    lv_textarea_set_max_length(log_box, 32767);              // Allow large buffer
    lv_textarea_set_text(log_box, "");
    lv_obj_set_style_text_font(log_box, &lv_font_montserrat_10, LV_PART_MAIN);
    lv_obj_set_style_bg_color(log_box, lv_color_black(), 0);
    lv_obj_set_style_text_color(log_box, lv_color_white(), 0);
    lv_obj_set_scrollbar_mode(log_box, LV_SCROLLBAR_MODE_OFF);
}

void updateDisplay() {
    lv_timer_handler();

    static uint32_t last_sec = 0;
    uint32_t total_sec = millis() / 1000;
    if (total_sec != last_sec && status_label) {
        last_sec = total_sec;

        uint32_t days = total_sec / 86400;
        uint32_t hours = (total_sec / 3600) % 24;
        uint32_t minutes = (total_sec / 60) % 60;
        uint32_t seconds = total_sec % 60;

        static char buf[64];
        if (days == 0) {
            snprintf(buf, sizeof(buf), "geogram uptime: %02u:%02u:%02u", hours, minutes, seconds);
        } else {
            snprintf(buf, sizeof(buf), "geogram uptime: %lu day%s %02u h",
                     days, (days == 1 ? "" : "s"), hours);
        }
        lv_label_set_text(status_label, buf);
    }
}
