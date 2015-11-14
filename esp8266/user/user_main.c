#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "user_interface.h"

static ETSTimer pollTimer;

static void ICACHE_FLASH_ATTR pollTimerCb(void *arg) {
  os_printf("Polling\n");
}

// WiFi event callback
static void ICACHE_FLASH_ATTR wifi_callback(System_Event_t *evt)
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
      break;

    case EVENT_STAMODE_GOT_IP:
      os_printf("IP: " IPSTR ", Mask: " IPSTR ", Gateway: " IPSTR "\n",
                IP2STR(&evt->event_info.got_ip.ip),
                IP2STR(&evt->event_info.got_ip.mask),
                IP2STR(&evt->event_info.got_ip.gw));
      break;
  }
}

// Init function 
void ICACHE_FLASH_ATTR user_init()
{
  struct station_config stationConf;

  // Enable GPIO
  gpio_init();

  // Set station mode
  wifi_set_opmode_current(STATION_MODE);

  // Set AP settings
  os_memcpy(&stationConf.ssid, SSID, 32);
  os_memcpy(&stationConf.password, SSID_PASSWORD, 64);
  wifi_station_set_config(&stationConf);

  // Set an event handler for WiFi events
  wifi_set_event_handler_cb(wifi_callback);

  // Start os task
  os_timer_disarm(&pollTimer);
  os_timer_setfn(&pollTimer, pollTimerCb, NULL);
  os_timer_arm(&pollTimer, 5000, 1);

  os_printf("user_init() complete!\n\r");
}
