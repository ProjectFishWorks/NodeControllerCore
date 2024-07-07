#ifndef NodeControllerCore_H
#define NodeControllerCore_H
#endif

#include <Arduino.h>
#include "driver/twai.h"

//CAN BUS Pins
#define RX_PIN 6
#define TX_PIN 7


#define TX_QUEUE_LENGTH 10

#define RX_QUEUE_LENGTH 10

#define RX_TX_BLOCK_TIME (1 * portTICK_PERIOD_MS)

class NodeControllerCore
{
private:
    twai_message_t create_message(uint32_t id, uint64_t *data);
    void transmit_tx_queue(void *queue);
    void receive_to_rx_queue(void *queue);
    
    QueueHandle_t tx_queue;
    QueueHandle_t rx_queue; 

public:
    bool debug;

    std::function <void(uint32_t id, uint64_t *data)> onMessageReceived;

    void NodeControllerCore();
    bool Init();
    void sendMessage(uint32_t id, uint64_t *data);
};