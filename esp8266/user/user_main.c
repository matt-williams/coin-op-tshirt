#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "user_interface.h"
#include "httpclient.h"

#define ELGPIO 2

static ETSTimer el_timer;
static ETSTimer poll_timer;

static void ICACHE_FLASH_ATTR el_timer_callback(void* arg) {
  os_printf("Turning off EL\n");
  gpio_output_set(0, (1<<ELGPIO), (1<<ELGPIO), 0);
}

static void ICACHE_FLASH_ATTR poll_http_callback(char* response, int http_status, char* full_response) {
  os_printf("GET response has status %d and body \"%s\"\n", http_status, response);
  if (http_status == 200) {
    os_printf("Turning on EL\n");
    gpio_output_set((1<<ELGPIO), 0, (1<<ELGPIO), 0);
    os_timer_disarm(&el_timer);
    os_timer_arm(&el_timer, 10000, 0);
  }
}

static void ICACHE_FLASH_ATTR poll_timer_callback(void* arg) {
  os_printf("Polling\n");
  http_get("http://coin-op-tshirt.herokuapp.com/simplify/poll", "", poll_http_callback);
}

// WiFi event callback
static void ICACHE_FLASH_ATTR wifi_callback(System_Event_t* evt)
{
  os_printf( "Got WiFi event: %d\n", evt->event );

  switch (evt->event)
  {
    case EVENT_STAMODE_CONNECTED:
      os_printf("Connected to SSID %s, channel %d\n",
                evt->event_info.connected.ssid,
                evt->event_info.connected.channel);
      break;

    case EVENT_STAMODE_DISCONNECTED:
      os_printf("Disconnected from SSID %s, reason %d\n",
                evt->event_info.disconnected.ssid,
                evt->event_info.disconnected.reason);
      os_timer_disarm(&poll_timer);
      break;

    case EVENT_STAMODE_GOT_IP:
      os_printf("IP: " IPSTR ", Mask: " IPSTR ", Gateway: " IPSTR "\n",
                IP2STR(&evt->event_info.got_ip.ip),
                IP2STR(&evt->event_info.got_ip.mask),
                IP2STR(&evt->event_info.got_ip.gw));
      poll_timer_callback(NULL);
      os_timer_arm(&poll_timer, 2000, 1);
      break;
  }
}

// Init function 
void ICACHE_FLASH_ATTR user_init()
{
  struct station_config station_conf;

  // Enable GPIO
  gpio_init();
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
  gpio_output_set(0, 0, (1<<ELGPIO), 0);

  // Set station mode
  wifi_set_opmode_current(STATION_MODE);

  // Set AP settings
  os_memcpy(&station_conf.ssid, SSID, 32);
  os_memcpy(&station_conf.password, SSID_PASSWORD, 64);
  wifi_station_set_config(&station_conf);

  // Set an event handler for WiFi events
  wifi_set_event_handler_cb(wifi_callback);

  // Setup poll and EL timers, but don't start them yet
  os_timer_disarm(&poll_timer);
  os_timer_setfn(&poll_timer, poll_timer_callback, NULL);
  os_timer_disarm(&el_timer);
  os_timer_setfn(&el_timer, el_timer_callback, NULL);

  os_printf("user_init() complete!\n\r");
}
