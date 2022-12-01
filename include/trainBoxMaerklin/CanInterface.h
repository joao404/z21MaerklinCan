/*********************************************************************
 * CanInterface
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

#include "Helper/Observer.h"
#include <array>

namespace Can
{

    typedef struct
    {
        struct
        {
            // The order of these bits must match deprecated message flags for compatibility reasons
            uint32_t extd : 1;         /**< Extended Frame Format (29bit ID) */
            uint32_t rtr : 1;          /**< Message is a Remote Frame */
            uint32_t ss : 1;           /**< Transmit as a Single Shot Transmission. Unused for received. */
            uint32_t self : 1;         /**< Transmit as a Self Reception Request. Unused for received. */
            uint32_t dlc_non_comp : 1; /**< Message's Data length code is larger than 8. This will break compliance with ISO 11898-1 */
            uint32_t reserved : 27;    /**< Reserved bits */
        };
        uint32_t identifier;         /**< 11 or 29 bit identifier */
        uint8_t data_length_code;    /**< Data length code */
        std::array<uint8_t, 8> data; /**< Data bytes (not relevant in RTR frame) */
    } Message;
};

class CanInterface : public Observable<Can::Message>
{
public:
    CanInterface(){};
    virtual ~CanInterface(){};

    virtual void begin() = 0;

    virtual bool transmit(Can::Message &frame, uint16_t timeoutINms) = 0;

    virtual bool receive(Can::Message &frame, uint16_t timeoutINms) = 0;
};