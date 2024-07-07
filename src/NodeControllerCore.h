#ifndef NodeControllerCore_H
#define NodeControllerCore_H

#include <Arduino.h>
#include "driver/twai.h"

// Pins used
#define RX_PIN 6
#define TX_PIN 7

class NodeControllerCore
{
private:
public:
    bool Init();
};