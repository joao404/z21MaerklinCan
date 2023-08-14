/*********************************************************************
 * Z60 ESP
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
#include "trainBoxMaerklin/CanInterfaceEsp32.h"
#include "trainBoxMaerklin/MaerklinLocoManagment.h"
#include "z21/UdpInterfaceEsp32.h"
#include "z60.h"
#include "Can2Lan.h"
#include "Cs2DataParser.h"

#include <SPIFFS.h>
#include <sqlite3.h>

twai_timing_config_t timingConfig = TWAI_TIMING_CONFIG_250KBITS();
std::shared_ptr<CanInterfaceEsp32> canInterface = std::make_shared<CanInterfaceEsp32>(timingConfig, GPIO_NUM_4, GPIO_NUM_5);

const uint16_t hash{0};
const uint32_t serialNumber{0xFFFFFFF0};
const uint16_t swVersion{0x0142};
const int16_t z21Port{21105};

std::shared_ptr<UdpInterfaceEsp32> udpInterface = std::make_shared<UdpInterfaceEsp32>(30, z21Port, false);

z60 centralStation(hash, serialNumber, z21Interface::HwType::Z21_XL, swVersion, false, false, false);

Can2Lan *can2Lan;

MaerklinLocoManagment locoManagment(0x0, centralStation, centralStation.getStationList(), 15000, 3);

File lokomotiveCs2;

const char *data = "Callback function called";
char *zErrMsg{0};
uint16_t locoId{1};
uint16_t functionId{1};
sqlite3 *z21Database;

/**********************************************************************************/
void setup()
{
  Serial.begin(230000);

  // Start the filesystem
  if (!SPIFFS.begin(false))
  {
    Serial.println(F("Failed to mount file system"));
    while (1)
      ;
  }

  AutoConnectConfig configAutoConnect;

  String idOfEsp = String((uint32_t)(ESP.getEfuseMac() >> 32), HEX);
  while (idOfEsp.length() < 4)
  {
    idOfEsp += "0";
  }
  Serial.print(F("ID of chip: "));
  Serial.println(idOfEsp);

  configAutoConnect.ota = AC_OTA_BUILTIN;
  configAutoConnect.apid = "z60AP-" + idOfEsp;
  configAutoConnect.psk = idOfEsp + idOfEsp;
  configAutoConnect.apip = IPAddress(192, 168, 0, 111); // Sets SoftAP IP address
  configAutoConnect.gateway = IPAddress(192, 168, 0, 111); // Sets Gateway IP address
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

  sqlite3_initialize();

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
        lokomotiveCs2.print(F(
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
            ""));
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
        WebService::getInstance()->setLocoList(locoManagment.getLocoList());
        if (nullptr != data)
        {
          // Serial.println(data->c_str());
          lokomotiveCs2.print(data->c_str());
          // get locoinfo from data. Every file is only one loco
          // write loco data and functions to sqlite
          // String sql = "insert into functions(id,vehicle_id,button_type,shortcut,time,position,image_name,function,show_function_number,is_configured) values(1,2,0,'','0',0,'light',0,1,0)";

          // returns if no loco data is present (lokomotive)
          Cs2DataParser::LocoData locoData;
          if (Cs2DataParser::parseCs2ToLocoData(data, locoData))
          {
            auto callback = [](void *data, int argc, char **argv, char **azColName) -> int
            {
              for (int i = 1; i < argc; i++)
              {
                Serial.printf("%s|", argv[i]);
              }
              Serial.printf("\n");
              return 0;
            };

            std::string sql = "insert into vehicles(id,name,image_name,type,max_speed,address) values(" + std::to_string(locoId) + ",'" + locoData.name + "','" + locoData.name + ".png'," + "0,120," + std::to_string(locoData.adress) + ")";

            if (sqlite3_exec(z21Database, sql.c_str(), callback, (void *)data, &zErrMsg) != SQLITE_OK)
            {
              Serial.printf("SQL error: %s\n", zErrMsg);
              sqlite3_free(zErrMsg);
            }

            for (auto iterator = locoData.functionData.begin(); iterator != locoData.functionData.end(); ++iterator)
            {
              sql = "insert into functions(id,vehicle_id,button_type,shortcut,time,position,image_name,function,show_function_number,is_configured) values(" + std::to_string(functionId) + "," + std::to_string(locoId) + "," + std::to_string(iterator->buttonType) + ",'" + iterator->shortcut + "','0'," + std::to_string(iterator->function) + ",'" + iterator->imageName + "'," + std::to_string(iterator->function) + ",1,0)";
              if (sqlite3_exec(z21Database, sql.c_str(), callback, (void *)data, &zErrMsg) != SQLITE_OK)
              {
                Serial.printf("SQL error: %s | %s\n", zErrMsg, sql.c_str());
                sqlite3_free(zErrMsg);
              }
              else
              {
                functionId++;
              }
            }
            locoId++;
          }
        }
      };

      auto lambdaWriteFileResult = [](bool success)
      {
        Serial.println(success ? "Getting locos success" : "Getting locos failed");
        if (success)
        {
          auto callback = [](void *data, int argc, char **argv, char **azColName) -> int
          {
            for (int i = 1; i < argc; i++)
            {
              Serial.printf("%s|", argv[i]);
            }
            Serial.printf("\n");
            return 0;
          };
          if (sqlite3_exec(z21Database, "Select * from vehicles", callback, (void *)data, &zErrMsg) != SQLITE_OK)
          {
            Serial.printf("SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
          }

          if (sqlite3_exec(z21Database, "Select * from functions", callback, (void *)data, &zErrMsg) != SQLITE_OK)
          {
            Serial.printf("SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
          }
        }

        lokomotiveCs2.close();
        // close database
        sqlite3_close(z21Database);

        WebService::getInstance()->setLocoList(locoManagment.getLocoList());
        WebService::getInstance()->setLokomotiveAvailable(success);
        WebService::getInstance()->setTransmissionFinished(true);
      };

      WebService::getInstance()->setTransmissionFinished(false);
      WebService::getInstance()->setLokomotiveAvailable(false);
      // open sql database and delete current entries
      sqlite3_initialize();

      int sqliteResult = sqlite3_open("/spiffs/config/Loco.sqlite", &z21Database);
      if (0 != sqliteResult)
      {
        Serial.printf("Can't open database: %s\n", sqlite3_errmsg(z21Database));
      }
      else
      {
        Serial.println(F("Opened database successfully"));

        auto callback = [](void *data, int argc, char **argv, char **azColName) -> int
        {
          for (int i = 1; i < argc; i++)
          {
            Serial.printf("%s|", argv[i]);
          }
          Serial.printf("\n");
          return 0;
        };

        if (sqlite3_exec(z21Database, "delete from vehicles", callback, (void *)data, &zErrMsg) != SQLITE_OK)
        {
          Serial.printf("SQL error: %s\n", zErrMsg);
          sqlite3_free(zErrMsg);
        }
        if (sqlite3_exec(z21Database, "delete from functions", callback, (void *)data, &zErrMsg) != SQLITE_OK)
        {
          Serial.printf("SQL error: %s\n", zErrMsg);
          sqlite3_free(zErrMsg);
        }
      }
      lokomotiveCs2 = SPIFFS.open("/config/lokomotive.cs2", FILE_WRITE);
      if (!lokomotiveCs2)
      {
        Serial.println(F("ERROR failed to open lokomotive.cs2 for writing"));
      }
      else
      {
        Serial.println(F("Openend lokomotive.cs2"));
      }

      if (lokomotiveCs2 && (0 == sqliteResult))
      {
        locoId = 1;
        functionId = 1;
        locoManagment.getLokomotiveConfig(lambdaWriteFile, lambdaWriteFileResult);
      }
    };

    auto searchMotorolaFkt = []()
    { centralStation.searchLoco(z60::ProgrammingProtocol::Mm2_20Khz); };

    auto searchDccShortFkt = []()
    { centralStation.searchLoco(z60::ProgrammingProtocol::DccShort); };

    auto searchDccLongFkt = []()
    { centralStation.searchLoco(z60::ProgrammingProtocol::DccLong); };

    auto onConnectFkt = [](IPAddress &ipaddr)
    {
      Serial.printf("WiFi connected with %s, IP:%s\n", WiFi.SSID().c_str(), ipaddr.toString().c_str());
      if (nullptr != udpInterface.get())
      {
        udpInterface->activateStationBroadcast();
      }
    };

    webService->getAutoConnect().onConnect(onConnectFkt);

    webService->begin(configAutoConnect, deleteLocoConfigFkt, defaultLocoListFkt, programmingFkt, readingFkt, searchMotorolaFkt,
                      searchDccShortFkt, searchDccLongFkt, centralStation.getFoundLocoString());
  }

  if (nullptr != canInterface.get())
  {
    canInterface->begin();
  }

  if (!centralStation.setCanObserver(canInterface))
  {
    Serial.println(F("ERROR: No can interface defined"));
  }

  if (nullptr != udpInterface.get())
  {
    udpInterface->begin();
  }

  if (!centralStation.setUdpObserver(udpInterface))
  {
    Serial.println(F("ERROR: No udp interface defined"));
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
  locoManagment.cyclic();
  udpInterface->cyclic();
  centralStation.cyclic();
  // delayMicroseconds(1);
}