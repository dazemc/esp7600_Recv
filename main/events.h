#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "lora.h"

extern QueueHandle_t loraTXQueue;
extern QueueHandle_t loraRXQueue;
extern QueueHandle_t displayQueue;
extern QueueHandle_t serialQueue;
extern QueueHandle_t telemetryQueue;

enum EventType {
  EVENT_LORA_TX,
  EVENT_LORA_RX,
  EVENT_LORA_SEND,
  EVENT_LORA_RECV,
  EVENT_VOLTAGE,
  EVENT_WIFI,
};

struct Event {
  EventType type;
  union {
    LoRaPacket loraRecv;
    SendLoRaData loraSend;
  };
};
