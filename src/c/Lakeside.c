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
#define API 2
#define CONDITIONS  3
#define SUNRISE 4
#define SUNSET 5

// Persistent storage key
#define SETTINGS_KEY 123

// Define our settings struct
typedef struct ClaySettings {
  char api[255];
} ClaySettings;

// An instance of the struct
static ClaySettings settings;

// Save the settings to persistent storage
static void default_settings() {
  snprintf(settings.api, sizeof(settings.api), "%s", "");
}

// Save the settings to persistent storage
static void save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

// get the saved settings from persistent storage
static void get_settings() {
  default_settings();
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void send_settings_update_weather(){

  // Begin dictionary
  DictionaryIterator *iter;

  // Start the sync to the js
  AppMessageResult result = app_message_outbox_begin(&iter);

  if(result == APP_MSG_OK) {

    // This is to pull the settings info from cache and push it to the index.js 
    dict_write_cstring(iter, MESSAGE_KEY_API, settings.api);
   
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

  // Get weather update every 30 minutes
  if((tick_time->tm_min % 30 == 0)) {
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

}

static void window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  text_layer_destroy(r_time_layer);
}

static void update_bg(char weather, char day_night){

  //weather = 'F'; //MANUAL SET for testing various conditions
  //day_night = 'N';
  // Destroy bitmaps before setting them again
  gbitmap_destroy(s_weather_bitmap);
  gbitmap_destroy(s_ripple_bitmap);
  
  switch (day_night){
    case 'D':

      // set text colors, black text and clear background for day
      text_layer_set_text_color(s_time_layer, GColorBlack); // GColorBlack if night/sunset, GColorClear if day/sunrise

      text_layer_set_background_color(r_time_layer, GColorClear); // GColorBlack if night/sunset, GColorClear if day/sunrise
      text_layer_set_text_color(r_time_layer, GColorBlack); //GColorWhite if night/sunset, GColorBlack if day/sunrise

      s_ripple_bitmap = gbitmap_create_with_resource(RESOURCE_ID_RIPPLE_DAY); 
      
      switch(weather){    
        case 'R' : //RAIN
          s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_RAIN_DAY);
          break;
        case 'T' : //THUNDERSTORMS
          s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_THUNDERSTORMS_DAY);
          break;
        case 'S' : //SNOW
          s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SNOW_DAY);
          break;
        case 'D' : //CLEAR
          s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DAY);
          break;
        case 'C' : //CLOUDS
          s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CLOUD_DAY);
          break;
        case 'F' : //FUZZY
          s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FOG_DAY);
          break;
        case 'O' : //TORNADO
          s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_TORNADO_DAY);
          break;
        case 'L' : //LOADING
          s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_LOADING);
          break;
        case 'U' : //SUNRISE
          s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SUNSET);
          break;
        case 'E' : //SUNSET
          s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SUNSET);
          break;
        default : //DEFAULT
          s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_LOADING);
      }
      break;
    case 'N':

      // set text colors, black text and clear background for day
      text_layer_set_text_color(s_time_layer, GColorWhite); // GColorBlack if night/sunset, GColorClear if day/sunrise

      text_layer_set_background_color(r_time_layer, GColorBlack); // GColorBlack if night/sunset, GColorClear if day/sunrise
      text_layer_set_text_color(r_time_layer, GColorWhite); //GColorWhite if night/sunset, GColorBlack if day/sunrise

      s_ripple_bitmap = gbitmap_create_with_resource(RESOURCE_ID_RIPPLE_NIGHT); 

      switch(weather){    
        case 'R' : //RAIN
          s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_RAIN_NIGHT);
          break;
        case 'T' : //THUNDERSTORMS
          s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_THUNDERSTORMS_NIGHT);
          break;
        case 'S' : //SNOW
          s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SNOW_NIGHT);
          break;
        case 'D' : //CLEAR
          s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_NIGHT);
          break;
        case 'C' : //CLOUDS
          s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CLOUD_NIGHT);
          break;
        case 'F' : //FUZZY
          s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FOG_NIGHT);
          break;
        case 'O' : //TORNADO
          s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_TORNADO_NIGHT);
          break;
        case 'L' : //LOADING
          s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_LOADING);
          break;
        case 'U' : //SUNRISE
          s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SUNSET);
          break;
        case 'E' : //SUNSET
          s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SUNSET);
          break;
        default : //DEFAULT
          s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_LOADING);
      }
      break;
  }

  //layer_add_child(window_layer, text_layer_get_layer(r_time_layer));
  bitmap_layer_set_bitmap(s_weather_bitmap_layer, s_weather_bitmap);
  //layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  bitmap_layer_set_bitmap(s_ripple_layer, s_ripple_bitmap);
}

// BEGIN weather shenanigans
static void update_weather(DictionaryIterator *iterator) {

  // Store incoming information
  static char conditions_buffer[32];
  static char sunset_buffer[32];
  static char sunrise_buffer[32];

  int current_time = 0;
  int sunrise_time = 0;
  int sunset_time = 0;
  char conditions_switch = ' ';
  char daynight_switch = ' ';
  
  // Read tuples for data
  Tuple *conditions_tuple = dict_find(iterator, CONDITIONS);
  Tuple *sunrise_tuple = dict_find(iterator, SUNRISE);
  Tuple *sunset_tuple = dict_find(iterator, SUNSET);
  Tuple *api_tuple = dict_find(iterator, API);

  if (api_tuple){
    snprintf(settings.api, sizeof(settings.api), "%s", api_tuple->value->cstring);
  } 

  // We're done looking at the settings returned. let's save it for future use.
  save_settings();
  // If temp is available, use it. We may not have the conditions, but at least show the temp

  if (conditions_tuple) {
    snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring); 
  }

  if (sunrise_tuple) {
    snprintf(sunrise_buffer, sizeof(sunrise_buffer), "%d", (int)sunrise_tuple->value->int32);
    sunrise_time =  (int)sunrise_tuple->value->int32;
  }

  if (sunset_tuple) {
    snprintf(sunset_buffer, sizeof(sunset_buffer), "%d", (int)sunset_tuple->value->int32);
    sunset_time =  (int)sunset_tuple->value->int32;
  }


  current_time = (int)time(NULL);

  if (sunrise_time < current_time && current_time < sunset_time){
    daynight_switch = 'D';
  } else {
    daynight_switch = 'N';
  }

  if ((sunrise_time - (60*30)) < current_time && current_time < (sunrise_time + (60*30))){
    conditions_switch = 'U';
  }

  if ((sunset_time - (60*30)) < current_time && current_time < (sunset_time + (60*30))){
    conditions_switch = 'E';
  }

  
  APP_LOG(APP_LOG_LEVEL_INFO, "conditions_buffer %s", conditions_buffer);
  if (strlen(conditions_buffer)>0 && conditions_switch == ' ') {

    // string search conditions_buffer for key words, but only if there's a value...

    //Rain
    if ((strstr(conditions_buffer,"Rain")) != NULL){  
      conditions_switch = 'R';
    }  
    //ThunderStorms 
    if ((strstr(conditions_buffer,"ThunderStorms")) != NULL){
      conditions_switch = 'T';
    }
    //Snow
    if ((strstr(conditions_buffer,"Snow")) != NULL){
      conditions_switch = 'S';
    }
    //Clear
    if ((strstr(conditions_buffer,"Clear")) != NULL){
      conditions_switch = 'D';
    }
    //Clouds
    if ((strstr(conditions_buffer,"Clouds")) != NULL){
      conditions_switch = 'C';
    }
    //Fuzzy Atmosphere
    if (((strstr(conditions_buffer,"Mist")) != NULL)
        || ((strstr(conditions_buffer,"Smoke")) != NULL)
        || ((strstr(conditions_buffer,"Haze")) != NULL)
        || ((strstr(conditions_buffer,"Dust")) != NULL)
        || ((strstr(conditions_buffer,"Fog")) != NULL)
        || ((strstr(conditions_buffer,"Sand")) != NULL)
        || ((strstr(conditions_buffer,"Dust")) != NULL)
        || ((strstr(conditions_buffer,"Ash")) != NULL)
        || ((strstr(conditions_buffer,"Squal")) != NULL))
    {
      conditions_switch = 'F';
    }
    //Tornado
    if ((strstr(conditions_buffer,"Tornado")) != NULL)
    {
      conditions_switch = 'O';
    }
  }

  if (conditions_switch == ' ') {
    //We didn't have anything, but we had a value for conditions
    // This would only happen if OpenWeather added a new MAIN value in the json
    // let's just default to Clear, since we know that much
    conditions_switch = 'D';
  }

  APP_LOG(APP_LOG_LEVEL_INFO, "daynight_switch %c", daynight_switch);
  APP_LOG(APP_LOG_LEVEL_INFO, "conditions_switch %c", conditions_switch);
  update_bg(conditions_switch, daynight_switch);
}


static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // was this just a ready signal or a "GIVE ME WEATHER I"M HUNGRY NOW!" sign
  Tuple *ready_tuple = dict_find(iterator, READY);

  if (ready_tuple) {
    
    // This is just a ready signal, We'll go back to the JS program
    // and give it the API information
    send_settings_update_weather();
  } else {
    // otherwise, this is just a weather update for you
    update_weather(iterator);
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
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

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

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

  app_event_loop();
  deinit();
}
