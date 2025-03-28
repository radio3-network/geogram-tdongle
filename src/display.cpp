#include <TFT_eSPI.h>
#include <lvgl.h>
#include "lv_driver.h"
#include "pinconfig.h"

TFT_eSPI screen = TFT_eSPI();

void initDisplay() {
    screen.init();
    screen.setRotation(1);
    screen.fillScreen(TFT_BLACK);
    pinMode(TFT_LEDA_PIN, OUTPUT);
    digitalWrite(TFT_LEDA_PIN, 0);

    screen.setTextFont(1);
    screen.setTextColor(TFT_GREEN, TFT_BLACK);
    delay(1000);

    lvgl_init();

    // Apply dark theme
    lv_theme_t* dark = lv_theme_default_init(nullptr, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_GREY), true, LV_FONT_DEFAULT);
    lv_disp_set_theme(nullptr, dark);

    lv_obj_t* label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "geogram");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 4, 2);
}


void updateDisplay() {
    lv_timer_handler();
}