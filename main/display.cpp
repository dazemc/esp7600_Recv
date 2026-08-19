#include "display.h"
#include "events.h"

static i2c_master_bus_config_t bus_config = {
    .i2c_port = I2C_NUM_0,
    .sda_io_num = OLED_SDA,
    .scl_io_num = OLED_SCL,
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .glitch_ignore_cnt = 7,
    .intr_priority = 0,
    .trans_queue_depth = 0,
    .flags =
        {
            .enable_internal_pullup = true,
            .allow_pd = false,
        },
};

ssd1306_handle_t initDisplay() {
  i2c_master_bus_handle_t bus_handle;

  ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));

  ssd1306_config_t cfg{};

  cfg.bus = SSD1306_I2C;
  cfg.width = OLED_WIDTH;
  cfg.height = OLED_HEIGHT;

  cfg.iface.i2c.port = I2C_NUM_0;
  cfg.iface.i2c.addr = 0x3C;
  cfg.iface.i2c.rst_gpio = OLED_RST;

  ssd1306_handle_t disp;
  ESP_ERROR_CHECK(ssd1306_new_i2c(&cfg, &disp));
  return disp;
}

void displayTask(void *arg) {
  DisplayData *data = (DisplayData *)arg;
  Event event;
  ssd1306_handle_t disp = data->disp;
  int oledX = data->OledX;
  int oledY = data->OledY;
  while (true) {
    if (xQueueReceive(displayQueue, &event, portMAX_DELAY)) {
      switch (event.type) {
      case EVENT_LORA_SEND: {
        const char *prepend = "Sending;\nVoltage: ";
        snprintf(data->message, sizeof(data->message), "%s%.4f", prepend,
                 event.loraSend.voltage);
        ssd1306_clear(disp);
        ssd1306_draw_text(disp, oledX, oledY, data->message, true);
        ssd1306_display(disp);
        break;
      }
      case EVENT_LORA_RECV: {
        char message[128];
        const char *prepend = "Received;\nVoltage: ";
        snprintf(message, sizeof(message), "%s%.4f", prepend,
                 event.loraSend.voltage);
        ssd1306_clear(disp);
        ssd1306_draw_text(disp, oledX, oledY, message, true);
        ssd1306_display(disp);
        break;
      }
      default:
        ssd1306_clear(disp);
        ssd1306_draw_text(disp, oledX, oledY, "Waiting for packet..", true);
        ssd1306_display(disp);
        break;
      }
    }
  }
}
