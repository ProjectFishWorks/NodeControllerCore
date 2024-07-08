#include <NodeControllerCore.h>

NodeControllerCore::NodeControllerCore(){
}

twai_message_t NodeControllerCore::create_message(uint32_t id, uint64_t *data) {
  twai_message_t message;

  message.extd = 1;
  message.identifier = id;
  message.data_length_code = 8;
  memcpy(message.data, data, 8);
  return message;
}

bool NodeControllerCore::Init(std::function<void(uint16_t nodeID, uint16_t messageID, uint64_t data)> onMessageReceived)
{
  this->debug = debug;
  this->onMessageReceived = onMessageReceived;

    // Initialize configuration structures using macro initializers
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)TX_PIN, 
                                                              (gpio_num_t)RX_PIN,
                                                              TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  // Install TWAI driver
  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    Serial.println("Driver installed");
  } else {
    Serial.println("Failed to install driver");
    return false;
  }

  // Start TWAI driver
  if (twai_start() == ESP_OK) {
    Serial.println("Driver started");
  } else {
    Serial.println("Failed to start driver");
    return false;
  }

  // Reconfigure alerts to detect frame receive, Bus-Off error and RX queue full states
  uint32_t alerts_to_enable = TWAI_ALERT_RX_DATA | TWAI_ALERT_ERR_PASS | TWAI_ALERT_BUS_ERROR | TWAI_ALERT_RX_QUEUE_FULL | TWAI_ALERT_BUS_OFF;
  if (twai_reconfigure_alerts(alerts_to_enable, NULL) == ESP_OK) {
    Serial.println("CAN Alerts reconfigured");
  } else {
    Serial.println("Failed to reconfigure alerts");
    return false;
  }

  tx_queue = xQueueCreate(TX_QUEUE_LENGTH, sizeof(twai_message_t));

  rx_queue = xQueueCreate(RX_QUEUE_LENGTH, sizeof(twai_message_t));

  //Create task to receive messages to rx_queue
  xTaskCreate(this->start_receive_to_rx_queue_task, 
              "start_rx_task_impl", 
              2048, 
              this, 
              10, 
              NULL);

  xTaskCreate(this->start_rx_queue_event_task, 
              "start_rx_queue_event_task_impl", 
              2048, 
              this, 
              20, 
              NULL);

  //Create task to transmit messages from tx_queue
  xTaskCreate(&NodeControllerCore::transmit_tx_queue, 
              "transmit_tx_queue", 
              2048, 
              &tx_queue, 
              30, 
              NULL);


  return true;

}

void NodeControllerCore::transmit_tx_queue(void *queue) {

  twai_message_t message;

  while(1){
    if(xQueueReceive(*(QueueHandle_t *) queue, &message, RX_TX_BLOCK_TIME) == pdTRUE){

      if (twai_transmit(&message, 2000) == ESP_OK) {
        Serial.println("Message queued for transmission");
      } else {
        Serial.println("Failed to queue message for transmission");
        delay(1000);
    }
    } 
  }
}

void NodeControllerCore::start_receive_to_rx_queue_task(void* _this) {
  ((NodeControllerCore*)_this)->receive_to_rx_queue();
}

void NodeControllerCore::receive_to_rx_queue() {

  twai_message_t message;

  Serial.println("Receive to rx queue task created");

  while(1){

    // Check if alert happened
    uint32_t alerts_triggered;
    twai_read_alerts(&alerts_triggered, 0);
    twai_status_info_t twaistatus;
    twai_get_status_info(&twaistatus);

    // Handle alerts
    if (alerts_triggered & TWAI_ALERT_ERR_PASS) {
      Serial.println("Alert: TWAI controller has become error passive.");
    }
    if (alerts_triggered & TWAI_ALERT_BUS_ERROR) {
      Serial.println("Alert: A (Bit, Stuff, CRC, Form, ACK) error has occurred on the bus.");
      Serial.printf("Bus error count: %d\n", twaistatus.bus_error_count);

    }
    if (alerts_triggered & TWAI_ALERT_RX_QUEUE_FULL) {
      Serial.println("Alert: The RX queue is full causing a received frame to be lost.");
      Serial.printf("RX buffered: %d\t", twaistatus.msgs_to_rx);
      Serial.printf("RX missed: %d\t", twaistatus.rx_missed_count);
      Serial.printf("RX overrun %d\n", twaistatus.rx_overrun_count);
    }

    if (alerts_triggered & TWAI_ALERT_BUS_OFF) {
      Serial.println("Bus Off state");
    }

    bool receive = false;
    do {
      receive = twai_receive(&message, RX_TX_BLOCK_TIME);

      if (receive == ESP_OK) {
        xQueueSend(this->rx_queue, &message, 0);
      } else if(receive == ESP_ERR_INVALID_STATE || receive == ESP_ERR_INVALID_ARG) {
        Serial.println("Failed to receive message");
      }
    } while (receive != ESP_ERR_TIMEOUT);
  }
}

void NodeControllerCore::start_rx_queue_event_task(void* _this) {
  ((NodeControllerCore*)_this)->rx_queue_event();
}

void NodeControllerCore::rx_queue_event() {
  twai_message_t message;

  while(1){
    if(xQueueReceive(rx_queue, &message, RX_TX_BLOCK_TIME) == pdTRUE){
      uint64_t data = 0;
      memcpy(&data, message.data, 8);
      uint16_t nodeID = message.identifier >> 15;
      uint16_t messageID = message.identifier & 0x7FFF;
      this->onMessageReceived(nodeID,messageID, data);
    }
  }
}

void NodeControllerCore::sendMessage(uint16_t messageID, uint64_t *data) {
  uint32_t id = (this->nodeID << 15) | messageID;
  twai_message_t message = create_message(id, data);
  xQueueSend(tx_queue, &message, portMAX_DELAY);
}