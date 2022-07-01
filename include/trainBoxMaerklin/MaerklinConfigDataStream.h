/*********************************************************************
 * TrainBox Maerklin ConfigDataStream
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
#include "trainBoxMaerklin/MaerklinCanInterface.h"
#include "trainBoxMaerklin/MaerklinStationConfig.h"
#include <array>
#include <memory>
#include <vector>
#include <string>

class MaerklinConfigDataStream
{
    public:
        enum class DataType : uint8_t
        {
            Lokliste,
            Lokinfo,
            Loknamen,
            MagInfo,
            Lokdb
        };

public:
    // can interface is needed to request data over interface
    MaerklinConfigDataStream(MaerklinCanInterface &interface, std::vector<MaerklinStationConfig> &stationList);
    virtual ~MaerklinConfigDataStream();

    bool requestConfigData(DataType type, std::string *info, std::string* buffer);

    bool onConfigData(uint16_t hash, std::array<uint8_t, 8> data);

    bool onConfigDataStream(uint16_t hash, uint32_t streamlength, uint16_t crc);

    bool onConfigDataStream(uint16_t hash, uint32_t streamlength, uint16_t crc, uint8_t res);

    bool onConfigDataStream(uint16_t hash, std::array<uint8_t, 8> &data);

    bool onConfigDataSteamError(uint16_t hash);

    void setHash(uint16_t hash) { m_hash = hash; }

    // void setConfigDataStreamFeedbackFunc(void (*reportResultFunc)(std::vector<uint8_t>* data, uint16_t hash, bool success)){m_reportResultFunc = reportResultFunc;}

protected:
    bool m_configDataStreamStartReceived{false};

    uint16_t m_hash{0};

    uint16_t m_hashExpected{0};

    uint16_t m_crcExpected{0};

    uint32_t m_length{0};

    uint32_t m_lengthExpected{0};

    virtual void m_reportResultFunc(std::string* data, uint16_t hash, bool success) = 0;

private:
    uint16_t updateCRC(uint16_t CRC_acc, uint8_t CRC_input);

    MaerklinCanInterface &m_interface;

    std::vector<MaerklinStationConfig> &m_stationList;

    std::string *m_buffer{nullptr};

    // backup buffer to save packages which are received outside of running transmission
    std::string m_backupBuffer;

    // void (*m_reportResultFunc)(std::vector<uint8_t>* data, uint16_t hash, bool success){nullptr};

};