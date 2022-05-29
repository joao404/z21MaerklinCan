/*********************************************************************
 * Z21 ESP
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

#include <Arduino.h>
// For Debugging active flag and read out via serial:
#define DEBUG

#include <EEPROM.h>

#include "WebService.h"
#include "trainBoxMaerklin/CanInterface.h"
#include "trainBoxMaerklin/MaerklinLocoManagment.h"
#include "z60.h"
#include "Can2Lan.h"

#include <SPIFFS.h>

std::shared_ptr<CanInterface> canInterface = std::make_shared<CanInterface>();

const uint16_t hash{0};
const uint32_t serialNumber{0xFFFFFFF0};
const uint16_t swVersion{0x0142};
const int16_t z21Port{21105};
z60 centralStation(hash, serialNumber, z21Interface::HwType::Z21_XL, swVersion, z21Port, true, false, false);

Can2Lan *can2Lan;

MaerklinLocoManagment locoManagment(0x0, centralStation, centralStation.getStationList(), 15000, 3);

File lokomotiveCs2;

/**********************************************************************************/
void setup()
{
  Serial.begin(230000);

  // Start the filesystem
  SPIFFS.begin(false);

  AutoConnectConfig configAutoConnect;

  String idOfEsp = String((uint32_t)(ESP.getEfuseMac() >> 32), HEX);
  while (idOfEsp.length() < 4)
  {
    idOfEsp += "0";
  }
  Serial.printf("ID of chip: ");
  Serial.println(idOfEsp);

  configAutoConnect.ota = AC_OTA_BUILTIN;
  configAutoConnect.apid = "z60AP-" + idOfEsp;
  configAutoConnect.psk = idOfEsp + idOfEsp;
  configAutoConnect.apip = IPAddress(192, 168, 0, 111); // Sets SoftAP IP address
  configAutoConnect.netmask = IPAddress(255, 255, 255, 0);
  configAutoConnect.channel = random(1, 12);
  Serial.printf("Wifi Channel:%d\n", configAutoConnect.channel);
  configAutoConnect.title = "z60";
  configAutoConnect.beginTimeout = 15000;
  configAutoConnect.autoReset = false;

  configAutoConnect.homeUri = "/";

  // reconnect with last ssid in handleClient
  configAutoConnect.autoReconnect = true;
  configAutoConnect.reconnectInterval = 15;

  configAutoConnect.portalTimeout = 1;

  configAutoConnect.immediateStart = true;
  configAutoConnect.autoRise = true;
  configAutoConnect.retainPortal = true;

  WebService *webService = WebService::getInstance();

  if (nullptr != webService)
  {
    auto deleteLocoConfigFkt = []()
    {
      centralStation.deleteLocoConfig();
    };

    auto defaultLocoListFkt = []()
    {
      WebService::getInstance()->setTransmissionFinished(false);
      WebService::getInstance()->setLokomotiveAvailable(false);
      lokomotiveCs2 = SPIFFS.open("/config/lokomotive.cs2", FILE_WRITE);
      if (!lokomotiveCs2)
      {
        Serial.println("ERROR failed to open lokomotive.cs2 for writing");
        WebService::getInstance()->setLokomotiveAvailable(false);
      }
      else
      {
        lokomotiveCs2.print(
            "[lokomotive]\n"
            "version\n"
            " .minor=3\n"
            "session\n"
            " .id=1\n"
            "lokomotive\n"
            " .name=BR 86\n"
            " .icon=loco\n"
            " .dv=1\n"
            " .uid=0x35\n"
            " .adresse=0x35\n"
            " .typ=mm2_prg\n"
            " .mfxuid=0x0\n"
            " .symbol=2\n"
            " .av=6\n"
            " .bv=3\n"
            " .volume=25\n"
            " .velocity=0\n"
            " .richtung=-1\n"
            " .vmax=60\n"
            " .vmin=3\n"
            " .funktionen\n"
            " ..nr=0\n"
            " ..typ=1\n"
            " ..dauer=0\n"
            " ..wert=0\n"
            " .funktionen\n"
            " ..nr=1\n"
            " ..typ=0\n"
            " ..dauer=0\n"
            " ..wert=0\n"
            " .funktionen\n"
            " ..nr=2\n"
            " ..typ=9\n"
            " ..dauer=0\n"
            " ..wert=0\n"
            " .funktionen\n"
            " ..nr=3\n"
            " ..typ=0\n"
            " ..dauer=0\n"
            " ..wert=0\n"
            " .funktionen\n"
            " ..nr=4\n"
            " ..typ=8\n"
            " ..dauer=0\n"
            " ..wert=0\n"
            " .funktionen\n"
            " ..nr=5\n"
            " ..typ=0\n"
            " ..dauer=0\n"
            " ..wert=0\n"
            " .funktionen\n"
            " ..nr=6\n"
            " ..typ=0\n"
            " ..dauer=0\n"
            " ..wert=0\n"
            " .funktionen\n"
            " ..nr=7\n"
            " ..typ=0\n"
            " ..dauer=0\n"
            " ..wert=0\n"
            " .funktionen\n"
            " ..nr=8\n"
            " ..typ=0\n"
            " ..dauer=0\n"
            " ..wert=0\n"
            " .funktionen\n"
            " ..nr=9\n"
            " ..typ=0\n"
            " ..dauer=0\n"
            " ..wert=0\n"
            " .funktionen\n"
            " ..nr=10\n"
            " ..typ=0\n"
            " ..dauer=0\n"
            " ..wert=0\n"
            " .funktionen\n"
            " ..nr=11\n"
            " ..typ=0\n"
            " ..dauer=0\n"
            " ..wert=0\n"
            " .funktionen\n"
            " ..nr=12\n"
            " ..typ=0\n"
            " ..dauer=0\n"
            " ..wert=0\n"
            " .funktionen\n"
            " ..nr=13\n"
            " ..typ=0\n"
            " ..dauer=0\n"
            " ..wert=0\n"
            " .funktionen\n"
            " ..nr=14\n"
            " ..typ=0\n"
            " ..dauer=0\n"
            " ..wert=0\n"
            " .funktionen\n"
            " ..nr=15\n"
            " ..typ=0\n"
            " ..dauer=0\n"
            " ..wert=0\n"
            "");
        lokomotiveCs2.close();
        WebService::getInstance()->setLokomotiveAvailable(true);
      }
      WebService::getInstance()->setTransmissionFinished(true);
    };

    auto programmingFkt = [](bool result)
    { centralStation.setProgramming(result); };

    auto readingFkt = []()
    {
      auto lambdaWriteFile = [](std::string *data)
      {
        if (nullptr != data)
        { /*Serial.println(data->c_str());*/
          lokomotiveCs2.print(data->c_str());
        }
      };

      auto lambdaWriteFileResult = [](bool success)
      {
        Serial.println(success ? "Getting locos success" : "Getting locos failed");
        lokomotiveCs2.close();
        WebService::getInstance()->setLocoList(locoManagment.getLocoList());
        WebService::getInstance()->setLokomotiveAvailable(success);
        WebService::getInstance()->setTransmissionFinished(true);
      };

      WebService::getInstance()->setTransmissionFinished(false);
      WebService::getInstance()->setLokomotiveAvailable(false);
      lokomotiveCs2 = SPIFFS.open("/config/lokomotive.cs2", FILE_WRITE);
      if (!lokomotiveCs2)
      {
        Serial.println("ERROR failed to open lokomotive.cs2 for writing");
      }
      else
      {
        locoManagment.getLokomotiveConfig(lambdaWriteFile, lambdaWriteFileResult);
      }
    };

    webService->begin(configAutoConnect, deleteLocoConfigFkt, defaultLocoListFkt, programmingFkt, readingFkt);
  }

  if (nullptr != canInterface.get())
  {
    canInterface->begin();
  }

  if (!centralStation.setCanObserver(canInterface))
  {
    Serial.println("ERROR: No can interface defined");
  }

  centralStation.setLocoManagment(&locoManagment);

  centralStation.begin();

  can2Lan = Can2Lan::getCan2Lan();
  if (nullptr != can2Lan)
  {
    can2Lan->begin(canInterface, false, false);
  }

  Serial.println("OK"); // start - reset serial receive Buffer
}

/**********************************************************************************/
void loop()
{
  WebService *webService = WebService::getInstance();
  if (nullptr != webService)
  {
    webService->cyclic();
  }
  canInterface->cyclic();
  centralStation.cyclic();
  locoManagment.cyclic();
  delayMicroseconds(1);
}