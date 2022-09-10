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
#include <AsyncUDP.h>
#include "z21/UdpInterface.h"
#include <memory>

#define ActTimeIP 60  // Aktivhaltung einer IP für (sec./2)
#define interval 2000 // interval in milliseconds for checking IP aktiv state

class UdpInterfaceEsp32 : public UdpInterface
{
public:
  typedef struct // Rückmeldung des Status der Programmierung
  {
    IPAddress IP;
    byte time; // aktive Zeit
  } listofIP;

  UdpInterfaceEsp32(uint16_t maxNumberOfClients, int16_t port, boolean debug);
  virtual ~UdpInterfaceEsp32(){};

  void begin() override;
  void cyclic();

  bool transmit(UdpMessage &message) override;

  bool receive(UdpMessage &message) override;

protected:
  void handlePacket(uint8_t client, uint8_t *packet, size_t packetLength);

private:
  const int m_port;
  // WiFiUDP Udp;
  AsyncUDP m_Udp;
  std::unique_ptr<listofIP[]> m_mem;
  uint16_t m_maxNumberOfClients;
  byte m_countIP; // zähler für Eintragungen

  // will store last time of IP decount updated
  unsigned long m_IPpreviousMillis;

  byte addIP(IPAddress ip);
};
