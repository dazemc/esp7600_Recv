#include "display.h"
#include "events.h"
#include "lora.h"
#include "serial.h"
#include <Arduino.h>
#include <LoRa.h>
#include <ssd1306.h>
#include <string>

QueueHandle_t loraRXQueue = nullptr;
QueueHandle_t loraTXQueue = nullptr;
QueueHandle_t displayQueue = nullptr;
QueueHandle_t serialQueue = nullptr;

static DisplayData displayData{};

extern "C" void app_main() {
  initArduino();
  initSerial();
  ssd1306_handle_t disp = initDisplay();
  initLoRa();

  loraRXQueue = xQueueCreate(10, sizeof(Event));
  loraTXQueue = xQueueCreate(10, sizeof(Event));
  displayQueue = xQueueCreate(10, sizeof(Event));
  serialQueue = xQueueCreate(10, sizeof(Event));
  if (loraRXQueue == nullptr || displayQueue == nullptr ||
      serialQueue == nullptr || loraTXQueue == nullptr) {
    Serial.println("Failed to create event queue");
    abort();
  }

  int OledX = 0;
  int OledY = 0;
  displayData.disp = disp;
  displayData.OledX = OledX;
  displayData.OledY = OledY;

  xTaskCreate(receiveLoRaTask, "RX LoRa", 4096, NULL, 3, NULL);
  xTaskCreate(displayTask, "display", 4096, &displayData, 2, NULL);
  xTaskCreate(printSerialTask, "serial", 4096, NULL, 2, NULL);
}
