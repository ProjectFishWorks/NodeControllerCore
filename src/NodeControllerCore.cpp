#include "driver/twai.h"

class NodeControllerCore
{
private:
    /* data */
public:
    bool Init();
};

bool NodeControllerCore::Init()
{
    // Initialize configuration structures using macro initializers
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)TX_PIN, 
                                                              (gpio_num_t)RX_PIN,
                                                              TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  // Install TWAI driver
  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    #if DEBUG
      Serial.println("Driver installed");
    #endif
  } else {
    #if DEBUG
      Serial.println("Failed to install driver");
    #endif
    return false;
  }

  // Start TWAI driver
  if (twai_start() == ESP_OK) {
    #if DEBUG
      Serial.println("Driver started");
    #endif
  } else {
    #if DEBUG
      Serial.println("Failed to start driver");
    #endif
    return false;
  }

  // Reconfigure alerts to detect frame receive, Bus-Off error and RX queue full states
  uint32_t alerts_to_enable = TWAI_ALERT_RX_DATA | TWAI_ALERT_ERR_PASS | TWAI_ALERT_BUS_ERROR | TWAI_ALERT_RX_QUEUE_FULL | TWAI_ALERT_BUS_OFF;
  if (twai_reconfigure_alerts(alerts_to_enable, NULL) == ESP_OK) {
    #if DEBUG
      Serial.println("CAN Alerts reconfigured");
    #endif
    return true;
  } else {
    #if DEBUG
      Serial.println("Failed to reconfigure alerts");
    #endif
    return false;
}
