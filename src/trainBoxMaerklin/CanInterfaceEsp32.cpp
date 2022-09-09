/*********************************************************************
 * CanInterface
 *
 * Copyright (C) 2022 Marcel Maage
 *
 * This library is free software; you twai redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * LICENSE file for more details.
 */

#include "trainBoxMaerklin/CanInterfaceEsp32.h"
#include <Arduino.h>
#include <driver/twai.h>
#include <driver/gpio.h>
#include <esp_system.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

CanInterfaceEsp32::CanInterfaceEsp32()
{
}

CanInterfaceEsp32::~CanInterfaceEsp32()
{
}

void CanInterfaceEsp32::begin()
{
    Serial.println(F("Setting up TWAI..."));
    /* set TWAI pins and baudrate */
    twai_general_config_t general_config = {
        .mode = TWAI_MODE_NORMAL,
        .tx_io = (gpio_num_t)GPIO_NUM_4, // 11,//5,
        .rx_io = (gpio_num_t)GPIO_NUM_5, // 10,//4,
        .clkout_io = (gpio_num_t)TWAI_IO_UNUSED,
        .bus_off_io = (gpio_num_t)TWAI_IO_UNUSED,
        .tx_queue_len = 120,
        .rx_queue_len = 120,
        .alerts_enabled = TWAI_ALERT_ABOVE_ERR_WARN | TWAI_ALERT_ERR_PASS | TWAI_ALERT_BUS_OFF | TWAI_ALERT_BUS_RECOVERED |
                          TWAI_ALERT_RX_QUEUE_FULL | TWAI_ALERT_BUS_ERROR, // TWAI_ALERT_NONE,
        .clkout_divider = 0};
    twai_timing_config_t timing_config = TWAI_TIMING_CONFIG_250KBITS();
    twai_filter_config_t filter_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    esp_err_t error;

    error = twai_driver_install(&general_config, &timing_config, &filter_config);
    if (error == ESP_OK)
    {
        Serial.println(F("TWAI Driver installation success..."));
    }
    else
    {
        Serial.println(F("TWAI Driver installation fail..."));
        return;
    }

    // start TWAI driver
    error = twai_start();
    if (error == ESP_OK)
    {
        Serial.println(F("TWAI Driver start success..."));
    }
    else
    {
        Serial.println(F("TWAI Driver start FAILED..."));
        return;
    }
}

void CanInterfaceEsp32::cyclic()
{
    twai_message_t frame;
    while (twai_receive(&frame, 0) == ESP_OK)
    {
        notify(&frame);
    }
    errorHandling();
}

bool CanInterfaceEsp32::transmit(CanMessage &frame, uint16_t timeoutINms)
{
    twai_message_t twaiFrame;
    twaiFrame.extd = frame.extd;
    twaiFrame.ss = frame.ss;
    twaiFrame.identifier = frame.identifier;
    twaiFrame.data_length_code = frame.data_length_code;
#if TWAI_FRAME_MAX_DLC > 8
    memcpy(&twaiFrame.data[0], &frame.data[0], 8);
#else
    memcpy(&twaiFrame.data[0], &frame.data[0], TWAI_FRAME_MAX_DLC);
#endif
    // std::copy(std::begin(frame.data), std::end(frame.data), std::begin(twaiFrame.data));
    bool result{true};
    if (twai_transmit(&twaiFrame, pdMS_TO_TICKS(timeoutINms)) != ESP_OK)
    {
        result = false;
        errorHandling();
    }
    return result;
}

bool CanInterfaceEsp32::receive(CanMessage &frame, uint16_t timeoutINms)
{
    twai_message_t twaiFrame;
    bool result{false};
    if (twai_receive(&twaiFrame, pdMS_TO_TICKS(timeoutINms)) == ESP_OK)
    {
        frame.extd = twaiFrame.extd;
        frame.ss = frame.extd;
        frame.identifier = twaiFrame.identifier;
        frame.data_length_code = twaiFrame.data_length_code;
#if TWAI_FRAME_MAX_DLC > 8
        memcpy(&frame.data[0], &twaiFrame.data[0], 8);
#else
        memcpy(&frame.data[0], &twaiFrame.data[0], TWAI_FRAME_MAX_DLC);
#endif
        // std::copy(std::begin(twaiFrame.data), std::end(twaiFrame.data), std::begin(frame.data));
        result = true;
    }
    return result;
}

void CanInterfaceEsp32::errorHandling()
{
    uint32_t alerts;
    twai_read_alerts(&alerts, 0);
    if (alerts & TWAI_ALERT_ABOVE_ERR_WARN)
    {
        Serial.println(F("Surpassed Error Warning Limit"));
    }
    if (alerts & TWAI_ALERT_ERR_PASS)
    {
        Serial.println(F("Entered Error Passive state"));
    }
    if (alerts & TWAI_ALERT_BUS_OFF)
    {
        Serial.println(F("Bus Off state"));
        // Prepare to initiate bus recovery, reconfigure alerts to detect bus recovery completion
        // twai_reconfigure_alerts(TWAI_ALERT_BUS_RECOVERED, NULL);
        for (int i = 3; i > 0; i--)
        {
            Serial.print(F("Initiate bus recovery in"));
            Serial.println(i);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        twai_initiate_recovery(); // Needs 128 occurrences of bus free signal
        Serial.println(F("Initiate bus recovery"));
    }
    if (alerts & TWAI_ALERT_BUS_RECOVERED)
    {
        // Bus recovery was successful, exit control task to uninstall driver
        Serial.println(F("Bus Recovered"));
        if (twai_start() == ESP_OK)
        {
            Serial.println(F("TWAI Driver start success..."));
        }
    }
    if (alerts & TWAI_ALERT_RX_QUEUE_FULL)
    {
        Serial.println(F("RxFull"));
    }
    if (alerts & TWAI_ALERT_BUS_ERROR)
    {
        Serial.println(F("BusError"));
    }
}