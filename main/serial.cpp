#include "serial.h"
#include "events.h"
#include <Arduino.h>

void initSerial() {
  Serial.begin(BAUD);
  vTaskDelay(pdMS_TO_TICKS(100));
  Serial.println("Serial started");
}

void printSerialTask(void *arg) {
  Event event;

  while (true) {
    if (xQueueReceive(serialQueue, &event, portMAX_DELAY)) {
      switch (event.type) {
      case EVENT_LORA_SEND: {
        Serial.print("Sending;\nVoltage: ");
        Serial.println(event.loraSend.voltage);
        break;
      }
      case EVENT_LORA_RECV: {
        break;
      }
      case EVENT_VOLTAGE: {
        break;
      }
      case EVENT_WIFI: {
        break;
      }
      case EVENT_LORA_TX: {
        break;
      }
      case EVENT_LORA_RX: {
        Serial.print("RX received: ");
        Serial.println(event.loraSend.voltage);
        break;
      }
      }
    }
  }
}
