/*********************************************************************
 * TrainBox Maerklin Esp32
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

#include "trainBoxMaerklin/MaerklinCanInterfaceObserver.h"

ZCanInterfaceObserver::ZCanInterfaceObserver(word hash, bool debug)
    : MaerklinCanInterface(hash, debug)
{
}

ZCanInterfaceObserver::~ZCanInterfaceObserver()
{
  end();
}

bool ZCanInterfaceObserver::setCanObserver(std::shared_ptr<CanInterface> canInterface)
{
  m_canInterface = canInterface;
  return nullptr != m_canInterface;
}

void ZCanInterfaceObserver::begin()
{
  m_canInterface->attach(*this);

  MaerklinCanInterface::begin();
}

void ZCanInterfaceObserver::end()
{
}

void ZCanInterfaceObserver::update(Observable &observable, void *data)
{
  if (&observable == m_canInterface.get())
  {
    if (nullptr != data)
    {
      CanInterface::CanMessage *frame = static_cast<CanInterface::CanMessage *>(data);

      TrackMessage message;
      message.clear();
      message.command = (frame->identifier >> 17) & 0xff;
      message.hash = frame->identifier & 0xffff;
      message.response = bitRead(frame->identifier, 16);
      message.length = frame->data_length_code;
      message.data = frame->data;

#ifdef CAN_DEBUG
      if (m_debug)
      {
        Serial.print("==> ");
        Serial.print(frame->identifier, HEX);
        Serial.print(" ");
        Serial.println(message);
      }
#endif
      handleReceivedMessage(message);
    }
  }
}

bool ZCanInterfaceObserver::sendMessage(TrackMessage &message)
{
  CanInterface::CanMessage txFrame;

  message.hash = m_hash;

  txFrame.identifier = (static_cast<uint32_t>(message.prio) << 25) | (static_cast<uint32_t>(message.command) << 17) | (uint32_t)message.hash;
  txFrame.extd = 1;
  txFrame.ss = 1;
  txFrame.data_length_code = message.length;
  txFrame.data = message.data;

#ifdef CAN_DEBUG
  if (m_debug)
  {
    Serial.print("<== ");
    Serial.print(tx_frame.identifier, HEX);
    Serial.print(" ");
    Serial.println(message);
  }
#endif
  bool result{false};
  if (nullptr != m_canInterface.get())
  {
    result = m_canInterface->transmit(txFrame, 100u);
  }
  return result;
}

bool ZCanInterfaceObserver::receiveMessage(TrackMessage &message)
{
  CanInterface::CanMessage rxFrame;

  bool result{false};

  if (nullptr != m_canInterface.get())
  {
    (m_canInterface->receive(rxFrame, 200u) == ESP_OK);
  }

  if (result)
  {
    message.clear();
    message.prio = (rxFrame.identifier >> 25) & 0x0f;
    message.command = (rxFrame.identifier >> 17) & 0xff;
    message.hash = rxFrame.identifier & 0xffff;
    message.response = bitRead(rxFrame.identifier, 16);
    message.length = rxFrame.data_length_code;
    message.data = rxFrame.data;

#ifdef CAN_DEBUG
    if (m_debug)
    {
      Serial.print("==> ");
      Serial.println(message);
    }
#endif
  }

  return result;
}