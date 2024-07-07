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
    static void transmit_tx_queue(void *queue);

    //Pattern from:
    //https://stackoverflow.com/questions/45831114/c-freertos-task-invalid-use-of-non-static-member-function
    static void start_rx_task_impl(void* _this);
    void receive_to_rx_queue();
    
    QueueHandle_t tx_queue;

    QueueHandle_t rx_queue;

    std::function<void(uint32_t id, uint64_t data)> onMessageReceived;

public:
    bool debug;

    NodeControllerCore();
    bool Init(bool debug, std::function<void(uint32_t id, uint64_t data)> onMessageReceived);
    void sendMessage(uint32_t id, uint64_t *data);
};