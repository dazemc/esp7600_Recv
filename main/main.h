#pragma once

#include "display.h"
#include "events.h"
#include "lora.h"
#include "serial.h"
#include <Arduino.h>
#include <LoRa.h>
#include <ssd1306.h>

extern QueueHandle_t loraRXQueue;
extern QueueHandle_t loraTXQueue;
extern QueueHandle_t displayQueue;
extern QueueHandle_t serialQueue;

void initMain();
void initQueue();
void initTasks();
