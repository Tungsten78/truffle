#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"


#define MY_UUID { 0x8F, 0xB4, 0x9F, 0x6D, 0xCE, 0x87, 0x4E, 0x44, 0x9A, 0xBA, 0x3F, 0x4E, 0x9A, 0xD3, 0x76, 0x2A }
PBL_APP_INFO(MY_UUID,
             "TruffleShuffle", "",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_WATCH_FACE);

#define WIDTH 144
#define HEIGHT 168

#define BLOCK_HEIGHT 30

#define DELAY 100
#define ITERATIONS 5

Window window;
BmpContainer bitmap_container;
TextLayer layer;
AppTimerHandle timer_handle;

int index = 0;
char buffer[16];

int images[] = {RESOURCE_ID_IMAGE_0,RESOURCE_ID_IMAGE_1,RESOURCE_ID_IMAGE_2,RESOURCE_ID_IMAGE_3,RESOURCE_ID_IMAGE_4,RESOURCE_ID_IMAGE_5,RESOURCE_ID_IMAGE_6,RESOURCE_ID_IMAGE_7,RESOURCE_ID_IMAGE_8,RESOURCE_ID_IMAGE_9,RESOURCE_ID_IMAGE_10,RESOURCE_ID_IMAGE_11};

void update_image(int resourceId)
{
    layer_remove_from_parent(&bitmap_container.layer.layer);
    bmp_deinit_container(&bitmap_container);
    
    bmp_init_container(resourceId, &bitmap_container);
    bitmap_layer_set_alignment(&bitmap_container.layer, GAlignCenter);
    
    GRect frame = layer_get_frame(&bitmap_container.layer.layer);
    frame.origin.x = (WIDTH - frame.size.w)/2;
    frame.origin.y = (HEIGHT - frame.size.h - BLOCK_HEIGHT/2)/2;
    layer_set_frame(&bitmap_container.layer.layer, frame);
    
    layer_add_child(&window.layer, &bitmap_container.layer.layer);
}

void handle_timer(AppContextRef ctx, AppTimerHandle handle, uint32_t cookie) {
    (void)ctx;
    (void)handle;
    
    if (cookie == 1)
    {
        layer_remove_from_parent(&bitmap_container.layer.layer);
        update_image(images[index++%ARRAY_LENGTH(images)]);
        
        if (index < (int)ARRAY_LENGTH(images) * ITERATIONS)
        {
            timer_handle = app_timer_send_event(ctx, DELAY, 1);
        }
        else
        {
            index = 0;
        }
    }
}

void handle_init(AppContextRef ctx) {
    (void)ctx;

    window_init(&window, "Window Name");
    window_set_background_color(&window, GColorBlack);
    window_stack_push(&window, true /* Animated */);
    
    PblTm time;
    get_time(&time);
    string_format_time(buffer,ARRAY_LENGTH(buffer),clock_is_24h_style()?"%H:%M":"%I:%M",&time);
    
    text_layer_init(&layer, GRect(0,(HEIGHT-BLOCK_HEIGHT),WIDTH,BLOCK_HEIGHT));
    text_layer_set_text_color(&layer, GColorWhite);
    text_layer_set_background_color(&layer, GColorBlack);
    text_layer_set_overflow_mode(&layer, GTextOverflowModeWordWrap);
    text_layer_set_font(&layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_set_text_alignment(&layer,GTextAlignmentCenter);
    text_layer_set_text(&layer, buffer);
    
    layer_add_child(&window.layer, &layer.layer);
    
    resource_init_current_app(&APP_RESOURCES);
    update_image(images[0]);
}

void handle_deinit(AppContextRef ctx)
{
    (void)ctx;
    
    bmp_deinit_container(&bitmap_container);
    app_timer_cancel_event(ctx,1);
}

int min = -1;

void handle_second_tick(AppContextRef ctx, PebbleTickEvent *t)
{
    (void)ctx;
    
    string_format_time(buffer,ARRAY_LENGTH(buffer),clock_is_24h_style()?"%H:%M":"%I:%M",t->tick_time);
    text_layer_set_text(&layer, buffer);
    
    if (t->tick_time->tm_min != min)
    {
        timer_handle = app_timer_send_event(ctx, DELAY, 1);
        
        min = t->tick_time->tm_min;
    }
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .deinit_handler = &handle_deinit,
    .timer_handler = &handle_timer,
    .tick_info = {
      .tick_handler = &handle_second_tick,
      .tick_units = SECOND_UNIT
    }
  };
  app_event_loop(params, &handlers);
}
