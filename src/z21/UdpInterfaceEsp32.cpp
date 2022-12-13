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

#include "Arduino.h"
#include "z21/UdpInterfaceEsp32.h"

UdpInterfaceEsp32::UdpInterfaceEsp32(uint16_t maxNumberOfClients, int16_t port, boolean debug)
    : m_port(port),
      m_mem(new listofIP[maxNumberOfClients]),
      m_maxNumberOfClients(maxNumberOfClients),
      m_countIP(0),
      m_IPpreviousMillis(0)
{
}

void UdpInterfaceEsp32::begin()
{
  if (m_Udp.listen(m_port))
  {
    m_Udp.onPacket([this](AsyncUDPPacket packet)
                   {  
      uint8_t packetBuffer[packet.length() + 1];
      memcpy(packetBuffer, packet.data(), packet.length());
      packetBuffer[packet.length()] = 0;
      // Serial.print("=>");
      // Serial.print(packet.remoteIP());
      // for(int i=0;i<packet.length();i++)
      // {
      //   Serial.print(" ");
      //   Serial.print(packet.data()[i]);
      // }
      // Serial.print("\n");
      handlePacket(addIP(packet.remoteIP()), packetBuffer, packet.length()); });
  }
}

void UdpInterfaceEsp32::handlePacket(uint8_t client, uint8_t *packet, size_t packetLength)
{
  // completely transmitted to

  if (4 <= packetLength)
  {
    uint16_t index = 0;
    uint16_t length = 0;
    for (size_t left_size = packetLength; left_size > 3;left_size -= length, index += length)
    {
      length = (packet[index + 1] << 8) + packet[index];
      if ((left_size < length) || (0 == length))
      {
        break;
      }
      Udp::Message udpMessage{client, &(packet[index])};
      notify(&udpMessage);
    }
  }
}

bool UdpInterfaceEsp32::transmit(Udp::Message &message)
{
  // send data now via new interface using transmit function

  uint16_t len = message.data[0] + (message.data[1] << 8);
  if (message.client == 0x00)
  { // Broadcast
    // Serial.print("B");
    //   for(uint16_t i=0;i<len;i++)
    //   {
    //     Serial.print(" ");
    //     Serial.print(data[i]);
    //   }
    // Serial.print("\n");
    // if(0 == countIP)
    // {
    //   Udp.broadcastTo(data, len, port);//, TCPIP_ADAPTER_IF_AP);
    // }

    // for (byte s = 0; s < countIP; s++) {
    //   if (mem[s].time > 0) {
    //     Udp.writeTo(data, len, mem[s].IP, port);
    //   }
    // }
    // Serial.println("B");
    // m_Udp.broadcastTo(message.data, message.data[0], m_port)
    if (m_Udp.broadcastTo(message.data, len, m_port) == len) //, TCPIP_ADAPTER_IF_AP);
    {
      return true;
    }
  }
  else
  {
    // Serial.print("C ");
    // Serial.print(mem[client-1].IP);
    // for(uint16_t i=0;i<len;i++)
    // {
    //   Serial.print(" ");
    //   Serial.print(data[i]);
    // }
    // Serial.print("\n");
    if (m_Udp.writeTo(message.data, len, m_mem[message.client - 1].IP, m_port) == len)
    {
      return true;
    }
  }
  return false;
}

bool UdpInterfaceEsp32::receive(Udp::Message &message)
{
  return false;
}

/**********************************************************************************/
byte UdpInterfaceEsp32::addIP(IPAddress ip)
{
  // suche ob IP schon vorhanden?
  for (byte i = 0; i < m_countIP; i++)
  {
    if (m_mem[i].IP == ip)
    {
      m_mem[i].time = ActTimeIP; // setzte Zeit
      return i + 1;              // Rückgabe der Speicherzelle
    }
  }
  // nicht vorhanden!
  if (m_countIP >= m_maxNumberOfClients)
  {
    for (byte i = 0; i < m_countIP; i++)
    {
      if (m_mem[i].time == 0)
      { // Abgelaufende IP, dort eintragen!
        m_mem[i].IP = ip;
        m_mem[i].time = ActTimeIP; // setzte Zeit
        return i + 1;
      }
    }
    Serial.print("EE"); // Fail
    return 0;           // Fehler, keine freien Speicherzellen!
  }
  m_mem[m_countIP].IP = ip;          // eintragen
  m_mem[m_countIP].time = ActTimeIP; // setzte Zeit
  m_countIP++;                       // Zähler erhöhen
  return m_countIP;                  // Rückgabe
}

void UdpInterfaceEsp32::cyclic()
{
  // Nutzungszeit IP's bestimmen
  unsigned long currentMillis = millis();
  if (currentMillis - m_IPpreviousMillis > interval)
  {
    m_IPpreviousMillis = currentMillis;
    for (byte i = 0; i < m_countIP; i++)
    {
      if (m_mem[i].time > 0)
        m_mem[i].time--; // Zeit herrunterrechnen
    }
    // notifyz21InterfacegetSystemInfo(0); //SysInfo an alle BC Clients senden!
  }
  // notifyz21InterfacegetSystemInfo(0);
}
