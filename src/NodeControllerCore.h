#ifndef NodeControllerCore_H
#define NodeControllerCore_H
#endif

#include <Arduino.h>
#include "driver/twai.h"

//CAN BUS Pins
#define RX_PIN 7
#define TX_PIN 6


//Queue length for tx_queue and rx_queue
#define TX_QUEUE_LENGTH 10
#define RX_QUEUE_LENGTH 10

//Time to wait for a message to be received TODO: not sure what this does, set to one tick=1ms right now
#define RX_TX_BLOCK_TIME (1 * portTICK_PERIOD_MS)

class NodeControllerCore
{
private:
    //Creates a TWAI message
    twai_message_t create_message(uint32_t id, uint64_t *data);

    //Pattern from:
    //https://stackoverflow.com/questions/45831114/c-freertos-task-invalid-use-of-non-static-member-function
    //Helper functions to start receive_to_rx_queue_task
    static void start_receive_to_rx_queue_task(void* _this);

    //Receive messages from CAN bus and put them in rx_queue
    void receive_to_rx_queue();

    //Helper functions to start rx_queue_event_task
    static void start_rx_queue_event_task(void* _this);
    //Transmit messages from tx_queue to CAN bus
    static void transmit_tx_queue(void *queue);

    //Receive messages from rx_queue and call onMessageReceived function in the device code
    void rx_queue_event();
    
    //Queue to transmit messages
    QueueHandle_t tx_queue;

    //Queue to receive messages
    QueueHandle_t rx_queue;

    //Function to call when a message is received
    std::function<void(uint8_t nodeID, uint16_t messageID, uint64_t data)> onMessageReceived;

public:
    //TODO: Add a debug flag to print messages
    bool debug;

    //Constructor
    NodeControllerCore();

    //Initialize various the node controller core
    bool Init(std::function<void(uint8_t nodeID, uint16_t messageID, uint64_t data)> onMessageReceived, uint8_t nodeID);

    //Send a message to the CAN bus
    void sendMessage(uint16_t messageID, uint64_t *data);
    void sendMessage(uint16_t messageID, float *data);

    //Node ID of the node
    uint8_t nodeID = 0;

};