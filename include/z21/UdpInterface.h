/*********************************************************************
 * UdpInterface
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

namespace Udp
{
    typedef struct {
        uint8_t client;
        uint8_t *data;
    } Message;
};

class UdpInterface : public Observable<Udp::Message>
{
public:
    UdpInterface(){};
    virtual ~UdpInterface(){};

    virtual void begin() = 0;

    virtual bool transmit(Udp::Message &message) = 0;

    virtual bool receive(Udp::Message &message) = 0;
};