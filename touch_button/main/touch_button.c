/* Touch Sensor Example

   For other examples please check:
   https://github.com/espressif/esp-iot-solution/tree/master/examples

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_err.h"
#include "iot_touchpad.h"

/*!< Set real touch value below */
#define TOUCH_TRIGGER_VALUE 1050.0
#define TOUCH_NORMAL_VALUE 1000.0
#define TOUCH_SENSITIVITY ((TOUCH_TRIGGER_VALUE - TOUCH_NORMAL_VALUE) * 1.0 / TOUCH_NORMAL_VALUE)

#define TOUCH_PAD_NUM TOUCH_PAD_NUM9 /*!< Touch pad channel 8 is GPIO33(ESP32) / GPIO8(ESP32-S2) */

//#define OUTPUT_FALTER_DATA

const static char *TAG = "touch_button";

typedef enum {
    TOUCH_EVT_SPRING_PUSH,
    TOUCH_EVT_SPRING_RELEASE,
} touch_evt_type_t;

typedef struct {
    touch_evt_type_t type;
    union {
        struct {
            int idx;
        } single;
    };
} touch_evt_t;

static QueueHandle_t q_touch;

/*!< callback must quick enough */
static void spring_push_cb(void *arg)
{
    tp_handle_t tp_handle = (tp_handle_t) arg;
    touch_pad_t tp_num = iot_tp_num_get(tp_handle);
    touch_evt_t evt;
    evt.type = TOUCH_EVT_SPRING_PUSH;
    evt.single.idx = (int) tp_num;
    xQueueSend(q_touch, &evt, 0);
}

/*!< callback must quick enough */
static void spring_release_cb(void *arg)
{
    tp_handle_t tp_handle = (tp_handle_t) arg;
    touch_pad_t tp_num = iot_tp_num_get(tp_handle);
    touch_evt_t evt;
    evt.type = TOUCH_EVT_SPRING_RELEASE;
    evt.single.idx = (int) tp_num;
    xQueueSend(q_touch, &evt, 0);
}

static void touch_background_task(void *arg)
{
    QueueHandle_t q_touch = (QueueHandle_t)arg;
    touch_evt_t evt;

    while (1) {
        portBASE_TYPE ret = xQueueReceive(q_touch, &evt, portMAX_DELAY);

        if (ret == pdTRUE) {
            switch (evt.type) {
                case TOUCH_EVT_SPRING_PUSH:
                    ESP_LOGI(TAG, "push touch pad num %d", evt.single.idx);
                    /* user code */
                    break;

                case TOUCH_EVT_SPRING_RELEASE:
                    ESP_LOGW(TAG, "release touch pad num %d", evt.single.idx);
                    /* user code */
                    break;

                default:
                    /* user code */
                    break;
            }
        }
    }

    vTaskDelete(NULL);
}

void app_main(void)
{
    q_touch = xQueueCreate(10, sizeof(touch_evt_t));
    /*!< Creat a touch pad*/
    tp_handle_t m_tp_handle = iot_tp_create(TOUCH_PAD_NUM, TOUCH_SENSITIVITY);
    assert(m_tp_handle != NULL);
    esp_err_t ret = iot_tp_add_cb(m_tp_handle, TOUCHPAD_CB_PUSH, spring_push_cb, (void *)m_tp_handle);
    assert(ret == ESP_OK);
    ret = iot_tp_add_cb(m_tp_handle, TOUCHPAD_CB_RELEASE, spring_release_cb, (void *)m_tp_handle);
    assert(ret == ESP_OK);

    xTaskCreate(touch_background_task, "TP_BKG", 2048, (void *)q_touch, 5, NULL);
    uint16_t touch_value = 0;

    while (1) {

#ifdef OUTPUT_FALTER_DATA
        ret = iot_tp_read(m_tp_handle, &touch_value);

        if (ret != ESP_OK) {
            ESP_LOGE("FLT", "tp read failed");
            continue;
        }

        ESP_LOGD("FLT", "tp read = %u", touch_value);

#else
        ret = tp_read_raw(m_tp_handle, &touch_value);

        if (ret != ESP_OK) {
            ESP_LOGE("RAW", "tp read failed");
            continue;
        }

        ESP_LOGD("RAW", "tp read = %u", touch_value);
#endif

        vTaskDelay(10 / portTICK_RATE_MS);
    }

}
