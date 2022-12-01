/*********************************************************************
 * CanInterfaceEsp32
 *
 * Copyright (C) 2022 Marcel Maage
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * LICENSE file for more details.
 */

#pragma once

#include "trainBoxMaerklin/CanInterface.h"
#include <driver/twai.h>

class CanInterfaceEsp32 : public CanInterface
{
public:
    CanInterfaceEsp32(twai_timing_config_t timingConfig, gpio_num_t txPin, gpio_num_t rxPin);
    virtual ~CanInterfaceEsp32();

    void begin() override;

    void cyclic();

    bool transmit(Can::Message &frame, uint16_t timeoutINms) override;

    bool receive(Can::Message &frame, uint16_t timeoutINms) override;

private:
    twai_timing_config_t m_timingConfig;
    gpio_num_t m_txPin;
    gpio_num_t m_rxPin;

    void errorHandling();
};