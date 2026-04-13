/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

const int BTN_PIN_R = 28;
const int BTN_PIN_Y = 21;

const int LED_PIN_R = 5;
const int LED_PIN_Y = 10;

SemaphoreHandle_t xSemLed1;
SemaphoreHandle_t xSemLed2;

void btn_callback(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_FALL) {
        if (gpio == BTN_PIN_R) {
            xSemaphoreGiveFromISR(xSemLed1, 0);
        } else if (gpio == BTN_PIN_Y) {
            xSemaphoreGiveFromISR(xSemLed2, 0);
        }
    }
}

void led_1_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    bool blinking = false;
    int count = 0;
    bool led_state = false;

    while (true) {
        if (xSemaphoreTake(xSemLed1, 0) == pdTRUE) {
            blinking = !blinking;
            if (!blinking) {
                gpio_put(LED_PIN_R, 0);
                led_state = false;
                count = 0;
            }
        }

        if (blinking) {
            count++;
            if (count >= 10) {
                led_state = !led_state;
                gpio_put(LED_PIN_R, led_state ? 1 : 0);
                count = 0;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void led_2_task(void *p) {
    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);

    bool blinking = false;
    int count = 0;
    bool led_state = false;

    while (true) {
        if (xSemaphoreTake(xSemLed2, 0) == pdTRUE) {
            blinking = !blinking;
            if (!blinking) {
                gpio_put(LED_PIN_Y, 0);
                led_state = false;
                count = 0;
            }
        }

        if (blinking) {
            count++;
            if (count >= 10) {
                led_state = !led_state;
                gpio_put(LED_PIN_Y, led_state ? 1 : 0);
                count = 0;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

int main() {
    stdio_init_all();

    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);

    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, GPIO_IN);
    gpio_pull_up(BTN_PIN_Y);

    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true,
                                       &btn_callback);
    gpio_set_irq_enabled(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true);

    xSemLed1 = xSemaphoreCreateBinary();
    xSemLed2 = xSemaphoreCreateBinary();

    xTaskCreate(led_1_task, "LED1_Task", 256, NULL, 1, NULL);
    xTaskCreate(led_2_task, "LED2_Task", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (1) {}

    return 0;
}
