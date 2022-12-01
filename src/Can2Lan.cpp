/*********************************************************************
 * Can2Lan
 *
 * Copyright (C) 2022 Marcel Maage
 *
 * based on https://github.com/GBert/railroad/blob/master/can2udp/src/can2udp.c
 * by Gerhard Bertelsmann
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

#include "Can2Lan.h"

#include <algorithm>

Can2Lan *Can2Lan::m_can2LanInstance = nullptr;

Can2Lan *Can2Lan::getCan2Lan()
{
    if (nullptr == m_can2LanInstance)
    {
        m_can2LanInstance = new Can2Lan();
    }
    return m_can2LanInstance;
}

Can2Lan::Can2Lan()
    : m_debug(false),
      m_canDebug(false),
      m_localPortUdp(15731),
      m_localPortTcp(15731),
      m_destinationPortUdp(15730),
      m_minimumCmdIntervalINms(100)
{
}

Can2Lan::~Can2Lan()
{
}

void Can2Lan::begin(std::shared_ptr<CanInterface> canInterface, bool debug, bool canDebug, int localPortUdp, int localPortTcp, int destinationPortUdp)
{
    m_canInterface = canInterface;
    m_debug = debug;
    m_canDebug = canDebug;
    m_localPortUdp = localPortUdp;
    m_localPortTcp = localPortTcp;
    m_destinationPortUdp = destinationPortUdp;
    // Udp.begin(local_port);
    if (m_udpInterface.listen(m_localPortUdp))
    {
        m_udpInterface.onPacket([this](AsyncUDPPacket packet)
                                { handleUdpPacket(packet.data(), packet.length()); });
    }

    m_tcpInterface.reset(new AsyncServer(m_localPortTcp));
    if (nullptr != m_tcpInterface.get())
    {
        m_tcpInterface->onClient(&handleNewTcpClient, m_tcpInterface.get());
        m_tcpInterface->begin();
    }

    if (nullptr == m_canInterface.get())
    {
        Serial.println("ERROR m_canInterface is nullptr");
        return;
    }
    m_canInterface->attach(*this);
    // send magic start frame
    Can::Message frame;
    frame.identifier = 0x360301UL;
    frame.extd = 1;
    frame.ss = 1;
    frame.data_length_code = 5;
    for (int i = 0; i < frame.data_length_code; i++)
    {
        frame.data[i] = 0;
    }
    frame.data[4] = 0x11;
    if (!m_canInterface->transmit(frame, 1000u))
    {
        Serial.println("ERROR CAN magic start write error");
    }
    Serial.println("Can2Lan setup finished");
}

// handle CAN frame
void Can2Lan::update(Observable<Can::Message> &observable, Can::Message *data)
{
    if (&observable == m_canInterface.get())
    {
        if (nullptr != data)
        {
            Can::Message *frame = data;

            uint8_t udpframe[16];
            memset(udpframe, 0, m_canFrameSize);
            uint32_t canid = htonl(frame->identifier);
            memcpy(udpframe, &canid, 4);
            udpframe[4] = frame->data_length_code;
            memcpy(&udpframe[5], &frame->data, frame->data_length_code);

            if (m_canDebug)
            {
                Serial.print("CAN ");
                Serial.print(frame->identifier, HEX);
                Serial.print(" ");
                Serial.print(frame->data_length_code, HEX);
                Serial.print(" ");
                for (int i = 0; i < (frame->data_length_code); i++)
                {
                    Serial.print(frame->data[i], HEX);
                    Serial.print(" ");
                }
                Serial.print("\n");
            }
            m_udpInterface.broadcastTo(udpframe, m_canFrameSize, m_destinationPortUdp); //, TCPIP_ADAPTER_IF_AP);
            // send to all Tcp clients
            Can2Lan *interface = Can2Lan::getCan2Lan();
            if (nullptr != interface)
            {
                for (auto finding = interface->m_tcpClients.begin(); finding != interface->m_tcpClients.end(); ++finding)
                {
                    if (((*finding)->space() > m_canFrameSize) && (*finding)->canSend())
                    {
                        (*finding)->add((const char *)udpframe, m_canFrameSize);
                        (*finding)->send();
                    }
                }
            }
        }
    }
}

void Can2Lan::handleUdpPacket(uint8_t *udpFrame, size_t size)
{
    // Maerklin UDP Format: always 13 bytes
    // byte 0-3 TWAI ID
    // byte 4 DLC
    // byte 5-12 TWAI data

    uint8_t tcpPackages{0};
    if (0 == (size % m_canFrameSize))
    {
        uint8_t numberOfMessages = size / m_canFrameSize;
        Can::Message txFrame;
        uint8_t *udpFramePtr = udpFrame;
        for (uint8_t index = 0; index < numberOfMessages;
             udpFramePtr = (udpFrame + (index * m_canFrameSize)), index++)
        {
            uint32_t canid = 0;
            memcpy(&canid, &udpFramePtr[0], 4);
            txFrame.identifier = ntohl(canid);
            txFrame.extd = 1;
            txFrame.ss = 1;
            txFrame.data_length_code = udpFramePtr[4];
            memcpy(&txFrame.data, &udpFramePtr[5], 8);

            if (m_canDebug)
            {
                Serial.print("UDP ");
                Serial.print(txFrame.identifier, HEX);
                Serial.print(" ");
                Serial.print(txFrame.data_length_code, HEX);
                Serial.print(" ");
                for (int i = 0; i < (txFrame.data_length_code); i++)
                {
                    Serial.print(txFrame.data[i], HEX);
                    Serial.print(" ");
                }
                Serial.print("\n");
            }

            // S88 event
            if ((txFrame.identifier & 0x00FF0000UL) == 0x00230000UL)
            {
                m_udpInterface.broadcastTo(udpFramePtr, m_canFrameSize, m_destinationPortUdp); //, TCPIP_ADAPTER_IF_AP);
                Can2Lan *interface = Can2Lan::getCan2Lan();
                if (nullptr != interface)
                {
                    for (auto finding = interface->m_tcpClients.begin(); finding != interface->m_tcpClients.end(); ++finding)
                    {
                        if (((*finding)->space() > m_canFrameSize) && (*finding)->canSend())
                        {
                            (*finding)->add((const char *)udpFramePtr, m_canFrameSize);
                            tcpPackages++;
                            // (*finding)->send();
                        }
                    }
                }
            }
            else
            {
                if (((txFrame.identifier & 0x00FF0000UL) == 0x00310000UL) &&
                    (txFrame.data[6] == 0xEE) && (txFrame.data[7] == 0xEE))
                {
                    if (m_debug)
                    {
                        Serial.println("CAN ping");
                    }
                    uint8_t udpframe_reply[16];
                    memset(udpframe_reply, 0, m_canFrameSize);
                    udpframe_reply[0] = 0x00;
                    udpframe_reply[1] = 0x30;
                    udpframe_reply[2] = 0x00;
                    udpframe_reply[3] = 0x00;
                    udpframe_reply[4] = 0x00;
                    m_udpInterface.broadcastTo(udpframe_reply, m_canFrameSize, m_destinationPortUdp); //, TCPIP_ADAPTER_IF_AP);
                    // ToDo: Send lokomotive.cs2 request to connected cs2
                }
                if (nullptr != m_canInterface)
                {
                    if (((txFrame.identifier & 0x00FF0000UL) == 0x000C0000UL) &&
                        (txFrame.data_length_code == 5))
                    {
                        // Block access for asking loko function value to prevent error in case of connected MS
                        continue;
                    }
                    if (((txFrame.identifier & 0x00FF0000UL) == 0x00080000UL) &&
                        (txFrame.data_length_code == 6)) // speed command received
                    {
                        // if speed is not set to zero, block to much speed commands
                        if ((txFrame.data[4] != 0) || (txFrame.data[5] != 0))
                        {
                            uint32_t adr = (txFrame.data[0] << 24) + (txFrame.data[1] << 16) + (txFrame.data[2] << 8) + txFrame.data[3];
                            auto isAdr = [&adr](const DataLoco &i)
                            { return i.adrTrainbox == adr; };
                            auto finding = std::find_if(m_dataLocos.begin(), m_dataLocos.end(), isAdr);
                            unsigned long currentTimeINms = millis();
                            if (finding != m_dataLocos.end())
                            {
                                if ((finding->lastSpeedCmdTimeINms + m_minimumCmdIntervalINms) < currentTimeINms)
                                {
                                    finding->lastSpeedCmdTimeINms = currentTimeINms;
                                }
                                else
                                {
                                    // last speed command was send a short while ago, ignore it
                                    continue;
                                }
                            }
                            else
                            {
                                m_dataLocos.emplace_back(DataLoco{adr, currentTimeINms});
                            }
                        }
                    }
                    if (!m_canInterface->transmit(txFrame, 500u))
                    {
                        if (m_debug)
                        {
                            Serial.println("CAN write error");
                        }
                    }
                }
            }
        }
    }
    if (tcpPackages > 0)
    {
        Can2Lan *interface = Can2Lan::getCan2Lan();
        if (nullptr != interface)
        {
            for (auto finding = interface->m_tcpClients.begin(); finding != interface->m_tcpClients.end(); ++finding)
            {
                if ((*finding)->canSend())
                {
                    (*finding)->send();
                }
            }
        }
    }
}

void Can2Lan::handleTcpPacket(void *arg, AsyncClient *client, void *data, size_t len)
{
    Can2Lan *interface = Can2Lan::getCan2Lan();
    if (nullptr != interface)
    {
        if (interface->m_debug)
        {
            Serial.printf("Data received from client %s \n", client->remoteIP().toString().c_str());
        }

        // Maerklin UDP Format: always 13 bytes
        // byte 0-3 TWAI ID
        // byte 4 DLC
        // byte 5-12 TWAI data
        uint8_t *tcpFrame = (uint8_t *)data;
        size_t size = len;
        uint8_t tcpPackages{0};
        if (0 == (size % interface->m_canFrameSize))
        {
            uint8_t numberOfMessages = size / interface->m_canFrameSize;
            Can::Message txFrame;
            uint8_t *tcpFramePtr = tcpFrame;
            for (uint8_t index = 0; index < numberOfMessages; index++)
            {
                tcpFramePtr = (tcpFrame + (index * interface->m_canFrameSize));
                uint32_t canid = 0;
                memcpy(&canid, &tcpFramePtr[0], 4);
                /* TWAI is stored in network Big Endian format */
                txFrame.identifier = ntohl(canid);
                txFrame.extd = 1;
                txFrame.ss = 1;
                txFrame.data_length_code = tcpFramePtr[4];
                memcpy(&txFrame.data, &tcpFramePtr[5], 8);

                if (interface->m_canDebug)
                {
                    Serial.print("TCP ");
                    Serial.print(txFrame.identifier, HEX);
                    Serial.print(" ");
                    Serial.print(txFrame.data_length_code, HEX);
                    Serial.print(" ");
                    for (int i = 0; i < (txFrame.data_length_code); i++)
                    {
                        Serial.print(txFrame.data[i], HEX);
                        Serial.print(" ");
                    }
                    Serial.print("\n");
                }

                // Can Device registration
                if ((txFrame.identifier & 0x00FF0000UL) == 0x00000000UL)
                {
                    if (tcpFramePtr[9] == 0x0C)
                    {
                        if (interface->m_debug)
                        {
                            Serial.println("Can device registration");
                        }
                        // TODO: posssible error based on BIG/Little Endian
                        tcpFramePtr[1] |= 1;
                        tcpFramePtr[4] = 7;
                        tcpFramePtr[10] = 0xff;
                        tcpFramePtr[11] = 0xff;
                        txFrame.identifier |= 0x00010000UL;
                        txFrame.data_length_code = 7;
                        txFrame.data[5] = 0xff;
                        txFrame.data[6] = 0xff;
                        if ((client->space() > interface->m_canFrameSize) && client->canSend())
                        {
                            client->add((const char *)tcpFramePtr, interface->m_canFrameSize);
                            tcpPackages++;
                            // (*finding)->send();
                        }
                    }
                }
                else if ((txFrame.identifier & 0x00FF0000UL) == 0x00400000UL)
                {
                    Serial.print("Requested config:");
                    for (int i = 0; i < (txFrame.data_length_code); i++)
                    {
                        Serial.print((char)txFrame.data[i]);
                    }
                    Serial.print("\n");
                    continue; // do not send over can or udp
                }
                interface->m_udpInterface.broadcastTo(tcpFramePtr, interface->m_canFrameSize, interface->m_destinationPortUdp); //, TCPIP_ADAPTER_IF_AP);

                if (nullptr != interface->m_canInterface)
                {
                    if (!interface->m_canInterface->transmit(txFrame, 1000u))
                    {
                        if (interface->m_debug)
                        {
                            Serial.println("CAN write error");
                        }
                    }
                }
            }
        }
        if (tcpPackages > 0)
        {
            Can2Lan *interface = Can2Lan::getCan2Lan();
            if (nullptr != interface)
            {
                for (auto finding = interface->m_tcpClients.begin(); finding != interface->m_tcpClients.end(); ++finding)
                {
                    if ((*finding)->canSend())
                    {
                        (*finding)->send();
                    }
                }
            }
        }
    }
}

void Can2Lan::handleNewTcpClient(void *arg, AsyncClient *client)
{
    Serial.printf("New Tcp client: %s\n", client->remoteIP().toString().c_str());
    // register events
    client->onData(&handleTcpPacket, NULL);
    client->onError(&handleError, NULL);
    client->onDisconnect(&handleDisconnect, NULL);
    client->onTimeout(&handleTimeOut, NULL);

    if ((client->space() > 13) && client->canSend())
    {
        uint8_t frame[13];
        memset(frame, 0, 13);
        uint32_t canid = htonl(0x00304711UL);
        memcpy(frame, &canid, 4);

        client->add((const char *)&frame, 13);
        client->send();
    }

    Can2Lan *interface = Can2Lan::getCan2Lan();
    if (nullptr != interface)
    {
        auto finding = std::find(interface->m_tcpClients.begin(), interface->m_tcpClients.end(), client);
        if (interface->m_tcpClients.end() == finding)
        {
            interface->m_tcpClients.push_back(client);
        }
    }
}

void Can2Lan::handleError(void *arg, AsyncClient *client, int8_t error)
{
    Serial.printf("Connection error %s from Tcp client %s \n", client->errorToString(error), client->remoteIP().toString().c_str());
}

void Can2Lan::handleDisconnect(void *arg, AsyncClient *client)
{
    Serial.printf("Tcp client disconnected\n");
    Can2Lan *interface = Can2Lan::getCan2Lan();
    if (nullptr != interface)
    {
        auto finding = std::find(interface->m_tcpClients.begin(), interface->m_tcpClients.end(), client);
        if (interface->m_tcpClients.end() != finding)
        {
            interface->m_tcpClients.erase(finding);
        }
    }
}

void Can2Lan::handleTimeOut(void *arg, AsyncClient *client, uint32_t time)
{
    Serial.printf("Tcp client ACK timeout ip: %s \n", client->remoteIP().toString().c_str());
    Can2Lan *interface = Can2Lan::getCan2Lan();
    if (nullptr != interface)
    {
        auto finding = std::find(interface->m_tcpClients.begin(), interface->m_tcpClients.end(), client);
        if (interface->m_tcpClients.end() != finding)
        {
            interface->m_tcpClients.erase(finding);
        }
    }
}