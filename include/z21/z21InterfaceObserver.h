/*********************************************************************
 * UdpInterfaceEsp32
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

#include <Arduino.h>
// #include <WiFiUDP.h>
#include "z21/z21Interface.h" 
#include "z21/UdpInterface.h"
#include "Helper/Observer.h"
#include <memory>

class z21InterfaceObserver:public z21Interface, public Observer<Udp::Message>
{
  public:
    z21InterfaceObserver(HwType hwType, uint32_t swVersion,  boolean debug);
    virtual ~z21InterfaceObserver(){};

    void begin();

        // set can observer for receiving and writing messages
    bool setUdpObserver(std::shared_ptr<UdpInterface> udpInterface);

// calls receive function of z21Interface
    virtual void update(Observable<Udp::Message> &observable, Udp::Message *data) override;

  protected:

	void notifyz21InterfaceEthSend(uint8_t client, uint8_t *data) override;

  private:
    std::shared_ptr<UdpInterface> m_udpInterface; 
};
