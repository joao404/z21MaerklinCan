#include "Arduino.h"
#include "z21/z21InterfaceObserver.h"

z21InterfaceObserver::z21InterfaceObserver(HwType hwType, uint32_t swVersion, boolean debug)
    : z21Interface(hwType, swVersion, debug)
{
}

bool z21InterfaceObserver::setUdpObserver(std::shared_ptr<UdpInterface> udpInterface)
{
  m_udpInterface = udpInterface;
  return nullptr != m_udpInterface;
}

void z21InterfaceObserver::begin()
{
  m_udpInterface->attach(*this);
}

void z21InterfaceObserver::update(Observable &observable, void *data)
{
  if (&observable == m_udpInterface.get())
  {
    if (nullptr != data)
    {
      UdpInterface::UdpMessage *message = static_cast<UdpInterface::UdpMessage *>(data);
      receive(message->client, message->data); // Auswertung
    }
  }
}

//--------------------------------------------------------------------------------------------
void z21InterfaceObserver::notifyz21InterfaceEthSend(uint8_t client, uint8_t *data)
{
  UdpInterface::UdpMessage message{client, data};
  m_udpInterface->transmit(message);
}
