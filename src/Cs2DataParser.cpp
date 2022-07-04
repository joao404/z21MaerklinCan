/*********************************************************************
 * TrainBox Maerklin CS2 Data Parser
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

#include "Cs2DataParser.h"

static const char *locoFunctionString[] = {
    " ",
    "light",          // Stirnbeleuchtung
    "interior_light", // Innenbeleuchtung
    "back_light",     // Rücklicht
    "light",          // Fernlicht
    "sound2",         // Geräusch
    "Pantograf",
    "steam", // Rauch
    "Rangiergang",
    "couple", // Telexkupplung beidseitig
    "bugle",  // Horn
    "Schaffnerpfiff",
    "whistle_short", // Pfeife
    "bell",          // Glocke
    "Links/Rechts",
    "Heben/Senken",
    "Drehen links",
    "Kranarm heben/senken",
    "ABV",
    "Pumpe",
    "Bremsenquietschen",
    "Schaltstufen",
    "Generator",
    "Betriebsgeräusch",
    "Motor",
    "Bahnhofsansage",
    "Kohle schaufeln",
    "Türen schließen",
    "Türe öffnen",
    "Lüftergeräusch",
    "Lüfter",
    "Feuerbüchse",
    "Innenbeleuchtung",
    "Tischlampe Ep. IV",
    "Tischlampe Ep.III",
    "Tischlampe Ep. II",
    "Schüttelrost",
    "Schienenstoß",
    "Nummernschild",
    "Betriebsgeräusch",
    "Zuglaufschild",
    "Führerstand hinten",
    "Führerstand vorn",
    "Kuppeln",
    "Pufferstoß",
    "Zugansage",
    "Kranhaken",
    "Blinklicht",
    "Führerstandsbel.",
    "Pressluft",
    "F0",
    "F1",
    "F2",
    "F3",
    "F4",
    "F5",
    "F6",
    "F7",
    "F8",
    "F9",
    "F10",
    "F11",
    "F12",
    "F13",
    "F14",
    "F15",
    "F16",
    "F17",
    "F18",
    "F19",
    "F20",
    "F21",
    "F22",
    "F23",
    "F24",
    "F25",
    "F26",
    "F27",
    "F28",
    "F29",
    "F30",
    "F31",
    "Telexkupplung hinten",
    "Telexkupplung vorne",
    "Pantograf hinten",
    "Pantograf vorne",
    "Licht hinten",
    "Licht vorne",
    "Heben",
    "Lüfter",
    "Triebwerksbeleuchtung",
    "Zylinder ausblasen",
    "Dampfstoß",
    "Kran",
    "Auf",
    "Ab",
    "Links",
    "Rechts",
    "Drehen rechts",
    "Magnet"};
bool Cs2DataParser::parseCs2ToLocoData(std::string *data, LocoData &locoData)
{
    bool result{false};
    // Serial.println("check string");
    // Serial.println(data->c_str());
    auto n = data->find("lokomotive\n");
    // text starts at beginning
    if (0 == n)
    {
        std::string::size_type pos = getParameter(data, ".name=", locoData.name);
        pos = getParameter(data, ".adresse=", locoData.adress);
        std::string adressType;
        pos = getParameter(data, ".typ=", adressType);
        if ("mm" == adressType || "mm2_prg" == adressType || "mmm2_lok" == adressType)
        {
            locoData.adress += 2000;
        }
        else if ("dcc")
        {
            locoData.adress += 8000;
        }
        else if ("mfx")
        {
            locoData.adress += 4000;
        }
        std::string::size_type startPos = pos;
        std::string::size_type funktionenLength = sizeof(".funktionen\n");
        for (pos = data->find(".funktionen\n", pos); std::string::npos != pos; pos = data->find(".funktionen\n", pos))
        {
            pos += funktionenLength;
            uint16_t fktNum = 0;
            if (std::string::npos == getParameter(data, "..nr=", fktNum, pos))
            {
                continue;
            }
            uint16_t fktType = 0;
            if (std::string::npos == getParameter(data, "..typ=", fktType, pos))
            {
                continue;
            }
            uint16_t fktTime = 0;
            getParameter(data, "..dauer=", fktTime, pos);
            // uint16_t fktValue = 0;
            // getParameter(data, "..wert=", fktValue, pos);
            if (0 != fktType)
            {
                locoData.functionData.emplace_back(FunctionData{locoFunctionString[fktType], fktNum, "", fktTime});
            }
            startPos = pos;
        }
        pos = startPos;
        funktionenLength = sizeof(".funktionen_2\n");
        for (pos = data->find(".funktionen_2\n", pos); std::string::npos != pos; pos = data->find(".funktionen_2\n", pos))
        {
            pos += funktionenLength;
            uint16_t fktNum = 0;
            if (std::string::npos == getParameter(data, "..nr=", fktNum, pos))
            {
                continue;
            }
            uint16_t fktType = 0;
            if (std::string::npos == getParameter(data, "..typ=", fktType, pos))
            {
                continue;
            }
            uint16_t fktTime = 0;
            getParameter(data, "..dauer=", fktTime, pos);
            // uint16_t fktValue = 0;
            // getParameter(data, "..wert=", fktValue, pos);
            if (0 != fktType)
            {
                locoData.functionData.emplace_back(FunctionData{locoFunctionString[fktType], fktNum, "", fktTime});
            }
        }
        result = true;
    }
    return result;
};

std::string::size_type Cs2DataParser::getParameter(std::string *data, std::string parameter, uint16_t &result, std::string::size_type startpos)
{
    std::string dummy;
    std::string::size_type rv = getParameter(data, parameter, dummy, startpos);
    if (!dummy.empty())
    {
        result = static_cast<uint16_t>(std::stoul(dummy, nullptr, 16));
    }
    return rv;
}

std::string::size_type Cs2DataParser::getParameter(std::string *data, std::string parameter, std::string &result, std::string::size_type startpos)
{
    std::string::size_type length = parameter.length();
    std::string::size_type start = data->find(parameter, startpos);
    if (std::string::npos != start)
    {
        std::string::size_type end = data->find('\n', start);
        if (std::string::npos != end)
        {
            result = data->substr(start + length, end - start - length);
            return end + 1;
        }
    }
    result = "";
    return std::string::npos;
}