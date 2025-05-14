#include <pebble.h>

static Window *s_window;

static TextLayer *s_time_layer;
static TextLayer *r_time_layer;

static BitmapLayer *s_ripple_layer;
static BitmapLayer *s_weather_bitmap_layer;

static GBitmap *s_ripple_bitmap;
static GBitmap *s_weather_bitmap;

static GFont s_font;

#define READY 1
#define WEATHERCODE  2
#define ISDAY 3
#define RANONCE 4

// Define our settings struct
typedef struct {
  int weatherCode;
  int isDay;
  int ranOnce;
} ClaySettings;

// An instance of the struct

static ClaySettings settings;

// Save the settings to persistent storage
static void default_settings() {
  settings.weatherCode = 100;
  settings.isDay = 5;
  settings.ranOnce = 0;
}

// Save the settings to persistent storage
static void save_settings() {
  persist_write_int(MESSAGE_KEY_WEATHERCODE, settings.weatherCode);
  persist_write_int(MESSAGE_KEY_ISDAY, settings.isDay);
  persist_write_int(MESSAGE_KEY_RANONCE, settings.ranOnce);
}

// get the saved settings from persistent storage
static void get_settings() {
  default_settings();
  settings.weatherCode = persist_read_int(MESSAGE_KEY_WEATHERCODE);
  settings.isDay = persist_read_int(MESSAGE_KEY_ISDAY);
  settings.ranOnce = persist_read_int(MESSAGE_KEY_RANONCE);
}

// BEGIN weather shenanigans
static void update_weather(DictionaryIterator *iterator, bool update_background_only) {

  // Store incoming information
  int weatherCode = 100;
  int isDay = 5;
  int ranOnce = 0;

  static GBitmap *local_weather_bitmap;
  static GBitmap *local_ripple_bitmap;
    
  if (!update_background_only) {
    // Read tuples for data
    Tuple *weatherCode_tuple = dict_find(iterator, WEATHERCODE);
    Tuple *isDay_tuple = dict_find(iterator, ISDAY);
    Tuple *ranOnce_tuple = dict_find(iterator, RANONCE);

    // If temp is available, use it. We may not have the weatherCode, but at least show the temp

    if (weatherCode_tuple) {
      weatherCode = (int)weatherCode_tuple->value->int32;
    }

    if (isDay_tuple) {
      isDay =  (int)isDay_tuple->value->int32;
    }
    if (ranOnce_tuple) {
      ranOnce =  (int)ranOnce_tuple->value->int32;
    }
    
    APP_LOG(APP_LOG_LEVEL_INFO, "update_weather isDay %d", isDay);
    APP_LOG(APP_LOG_LEVEL_INFO, "update_weather weatherCode %d", weatherCode);
    APP_LOG(APP_LOG_LEVEL_INFO, "update_weather ranOnce %d", ranOnce);
  }

  if (isDay != 1 && isDay != 0){
    isDay = settings.isDay;
  }

  if (weatherCode > 99 ){
    weatherCode = settings.weatherCode;
  }

  if (ranOnce == 0 && settings.ranOnce == 1){
    ranOnce = settings.ranOnce;
  }
  
  if (!ranOnce){
    // APP_LOG(APP_LOG_LEVEL_INFO, "AAAAAAAAAAAAAAAAAAAAAHHHHHHHHHHHHHHHHHH");
    return;
  }

  if (isDay == 0){
    // set text colors, black text and clear background for day
    text_layer_set_text_color(s_time_layer, GColorWhite); // GColorBlack if night/sunset, GColorClear if day/sunrise

    text_layer_set_background_color(r_time_layer, GColorBlack); // GColorBlack if night/sunset, GColorClear if day/sunrise
    text_layer_set_text_color(r_time_layer, GColorWhite); //GColorWhite if night/sunset, GColorBlack if day/sunrise

    local_ripple_bitmap = gbitmap_create_with_resource(RESOURCE_ID_RIPPLE_NIGHT); 
  } 
  
  if (isDay == 1){
    // set text colors, black text and clear background for day
    text_layer_set_text_color(s_time_layer, GColorBlack); // GColorBlack if night/sunset, GColorClear if day/sunrise

    text_layer_set_background_color(r_time_layer, GColorWhite); // GColorBlack if night/sunset, GColorClear if day/sunrise
    text_layer_set_text_color(r_time_layer, GColorBlack); //GColorWhite if night/sunset, GColorBlack if day/sunrise

    local_ripple_bitmap = gbitmap_create_with_resource(RESOURCE_ID_RIPPLE_DAY); 
  }
  
  //Rain
  if (weatherCode == 51 
    || weatherCode == 53 
    || weatherCode == 55 
    || weatherCode == 56 
    || weatherCode == 57
    || weatherCode == 61
    || weatherCode == 63 
    || weatherCode == 65 
    || weatherCode == 66
    || weatherCode == 67 
    || weatherCode == 80 
    || weatherCode == 81 
    || weatherCode == 82){
    if (isDay == 0){
      local_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_RAIN_NIGHT);
    } else {
      local_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_RAIN_DAY);
    }
  }
  //ThunderStorms 
  if (weatherCode == 95 
      || weatherCode == 96 
      || weatherCode == 99){
    if (isDay == 0){
      local_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_THUNDERSTORMS_NIGHT);
    } else {
      local_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_THUNDERSTORMS_DAY);
    }
  }
  //Snow
  if (weatherCode == 71 
    || weatherCode == 73 
    || weatherCode == 75 
    || weatherCode == 77
    || weatherCode == 85 
    || weatherCode == 86){
    if (isDay == 0){
      local_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SNOW_NIGHT);
    } else {
      local_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SNOW_DAY);
    }
  }
  //Clear
  if (weatherCode == 0){
    if (isDay == 0){
      local_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_NIGHT);
    } else {
      local_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DAY);
    }
  }
  //Clouds
  if (weatherCode == 1
    || weatherCode == 2
    || weatherCode == 3){
    if (isDay == 0){
      local_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CLOUD_NIGHT);
    } else {
      local_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CLOUD_DAY);
    }
  }
  //Fuzzy Atmosphere
  if (weatherCode == 45
    || weatherCode == 48){
    if (isDay == 0){
      local_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FOG_NIGHT);
    } else {
      local_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FOG_DAY);
    }
  }

  APP_LOG(APP_LOG_LEVEL_INFO, "update_weather isDay %d", isDay);
  APP_LOG(APP_LOG_LEVEL_INFO, "update_weather weatherCode %d", weatherCode);
  
  if (weatherCode < 100){
    settings.weatherCode = weatherCode;
  }

  if (isDay == 0 || isDay == 1){
    settings.isDay = isDay;
  }

  if (ranOnce != 0){
    settings.ranOnce = ranOnce;
  }

  if ((settings.isDay == 0 || settings.isDay == 1) && settings.weatherCode < 100){
    gbitmap_destroy(s_weather_bitmap);
    s_weather_bitmap = local_weather_bitmap;

    gbitmap_destroy(s_ripple_bitmap);
    s_ripple_bitmap = local_ripple_bitmap;

    // We're done looking at the settings returned. let's save it for future use.
    save_settings();
    
    bitmap_layer_set_bitmap(s_weather_bitmap_layer, s_weather_bitmap);
    //layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_weather_bitmap_layer));

    bitmap_layer_set_bitmap(s_ripple_layer, s_ripple_bitmap);
    //layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_ripple_layer));
  }
}

static void send_settings_update_weather(){

  // Begin dictionary
  DictionaryIterator *iter = NULL;
  
  // This is to pull the settings info from cache and push it to the index.js 
  dict_write_int(iter, MESSAGE_KEY_WEATHERCODE, &settings.weatherCode, sizeof(int), true);
  dict_write_int(iter, MESSAGE_KEY_ISDAY, &settings.isDay, sizeof(int), true);
  dict_write_int(iter, MESSAGE_KEY_RANONCE, &settings.ranOnce, sizeof(int), true);

  // Start the sync to the js
  AppMessageResult result = app_message_outbox_begin(&iter);

  if(result == APP_MSG_OK) {


    // Send this message
    result = app_message_outbox_send();

    // Check the resultW
    if(result != APP_MSG_OK) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the outbox: %d", (int)result);
    }

  } else {
    // The outbox cannot be used right now
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox: %d", (int)result);
  }

  save_settings();
  update_weather(iter, false);
}

static void update_time() {

  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
 
  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
 
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
  text_layer_set_text(r_time_layer, s_buffer);
  
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  
  update_time();
  
  // Get weather update every 30min
  if ((tick_time->tm_min % 30 == 0) 
      || settings.weatherCode > 99 
      || (settings.isDay != 0 && settings.isDay != 1)
      || settings.ranOnce == 0) {

    APP_LOG(APP_LOG_LEVEL_INFO, "tick_handler settings.weatherCode %d", settings.weatherCode);
    APP_LOG(APP_LOG_LEVEL_INFO, "tick_handler tick_time->tm_hr %d", tick_time->tm_hour);
    send_settings_update_weather();
  }
}

static void window_load(Window *window) {

  Layer *window_layer = window_get_root_layer(window);
  GRect fullscreen = layer_get_bounds(window_layer);

  // Time Layer Reflection
  r_time_layer = text_layer_create(GRect(0, fullscreen.size.h/2, fullscreen.size.w, fullscreen.size.h));
  text_layer_set_text(r_time_layer, "00:00");
  s_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ATWRITER_REFLECT_63));
  text_layer_set_font(r_time_layer, s_font);
  text_layer_set_text_alignment(r_time_layer, GTextAlignmentCenter);
  text_layer_set_background_color(r_time_layer, GColorClear); // GColorBlack if night/sunset, GColorClear if day/sunrise
  text_layer_set_text_color(r_time_layer, GColorBlack); //GColorWhite if night/sunset, GColorBlack if day/sunrise
  layer_add_child(window_layer, text_layer_get_layer(r_time_layer));

  // weather
  s_weather_bitmap =gbitmap_create_with_resource(RESOURCE_ID_LOADING);
  s_weather_bitmap_layer = bitmap_layer_create(GRect(0, -fullscreen.size.h/9, fullscreen.size.w, fullscreen.size.h));
  bitmap_layer_set_compositing_mode(s_weather_bitmap_layer, GCompOpSet);
  // bitmap_layer_set_compositing_mode(s_weather_bitmap_layer, GCompOpAssignInverted);
  bitmap_layer_set_bitmap(s_weather_bitmap_layer, s_weather_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_weather_bitmap_layer));
  
  
  // Time Layer Top
  s_time_layer = text_layer_create(GRect(0, fullscreen.size.h/3, fullscreen.size.w, fullscreen.size.h));
  text_layer_set_text(s_time_layer, "00:00");
  s_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ATWRITER_63));
  text_layer_set_font(s_time_layer, s_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack); //GColorWhite if night/sunset, GColorBlack if day/sunrise
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  // Ripple Layer
  s_ripple_bitmap =gbitmap_create_with_resource(RESOURCE_ID_RIPPLE_DAY); //RESOURCE_ID_RIPPLE_NIGHT if night/sunset, RESOURCE_ID_RIPPLE_DAY if day/sunrise
  s_ripple_layer = bitmap_layer_create(GRect(0, fullscreen.size.h/10, fullscreen.size.w, fullscreen.size.h));
  bitmap_layer_set_compositing_mode(s_ripple_layer, GCompOpSet);
  // bitmap_layer_set_compositing_mode(s_ripple_layer, GCompOpAssignInverted);
  bitmap_layer_set_bitmap(s_ripple_layer, s_ripple_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_ripple_layer));

  
  DictionaryIterator *iter = NULL;
  update_weather(iter, true);
  // Make sure the time is displayed from the start
  update_time();
}

static void window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  text_layer_destroy(r_time_layer);
}


static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  
  // was this just a ready signal or a "GIVE ME WEATHER I"M HUNGRY NOW!" sign
  Tuple *ready_tuple = dict_find(iterator, READY);
  
  if (!ready_tuple) {
    // otherwise, this is just a weather update for you
    update_weather(iterator, false);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "inbox_dropped_callback Message dropped! Reason: %d", (int)reason);
}

static void outbox_failed_callback(DictionaryIterator *iter,
                                      AppMessageResult reason, void *context) {
  // The message just sent failed to be delivered
  APP_LOG(APP_LOG_LEVEL_ERROR, "outbox_failed_callback Message send failed. Reason: %d", (int)reason);
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "outbox_sent_callback Outbox send success!");
}

static void init(void) {
  
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  
  // Get any saved storage
  get_settings();
    
  const bool animated = true;
  window_stack_push(s_window, animated);
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  const int inbox_size = 128;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);
  
}

static void deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  init();

  app_event_loop();
  deinit();
}
