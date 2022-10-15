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

static const std::vector<Cs2DataParser::LocoFunctionType> functionType = {
    {" ", " ", 0},
    {"Licht", "light", 0},                     // Stirnbeleuchtung
    {"Innenbeleuchtung", "interior_light", 0}, // Innenbeleuchtung
    {"Rücklicht", "back_light", 0},            // Rücklicht
    {"Fernlicht", "light", 0},                 // Fernlicht
    {"Geräusch", "sound1", 0},                 // Geräusch
    {"Pantograf", "", 0},
    {"Rauch", "steam", 0},           // Rauch
    {"Rangiergang", "hump_gear", 0}, // Rangiergang
    {"Telexkupplung", "couple", 0},  // Telexkupplung beidseitig
    {"Horn", "bugle", 1},            // Horn
    {"Schaffnerpfiff", "", 1},
    {"Pfeife", "whistle_short", 1}, // Pfeife
    {"Glocke", "bell", 1},          // Glocke
    {"Links/Rechts", "", 1},
    {"Heben/Senken", "", 1},
    {"Drehen links", "", 1},
    {"Kranarm heben/senken", "", 1},
    {"ABV", "", 0},
    {"Pumpe", "", 0},
    {"Bremsenquietschen", "", 0},
    {"Schaltstufen", "", 0},
    {"Generator", "", 0},
    {"Betriebsgeräusch", "", 0},
    {"Motor", "", 0},
    {"Bahnhofsansage", "", 1},
    {"Kohle schaufeln", "", 0},
    {"Türen schließen", "", 1},
    {"Türe öffnen", "", 1},
    {"Lüftergeräusch", "", 0},
    {"Lüfter", "", 0},
    {"Feuerbüchse", "", 0},
    {"Innenbeleuchtung", "", 0},
    {"Tischlampe Ep. IV", "", 0},
    {"Tischlampe Ep.III", "", 0},
    {"Tischlampe Ep. II", "", 0},
    {"Schüttelrost", "", 0},
    {"Schienenstoß", "", 0},
    {"Nummernschild", "", 0},
    {"Betriebsgeräusch", "", 0},
    {"Zuglaufschild", "", 0},
    {"Führerstand hinten", "", 0},
    {"Führerstand vorn", "", 0},
    {"Kuppeln", "couple", 0}, // Kuppeln
    {"Pufferstoß", "", 0},
    {"Zugansage", "", 0},
    {"Kranhaken", "", 0},
    {"Blinklicht", "", 0},
    {"Führerstandsbel.", "", 0},
    {"Pressluft", "", 0},
    {"F0", "", 0},
    {"F1", "", 0},
    {"F2", "", 0},
    {"F3", "", 0},
    {"F4", "", 0},
    {"F5", "", 0},
    {"F6", "", 0},
    {"F7", "", 0},
    {"F8", "", 0},
    {"F9", "", 0},
    {"F10", "", 0},
    {"F11", "", 0},
    {"F12", "", 0},
    {"F13", "", 0},
    {"F14", "", 0},
    {"F15", "", 0},
    {"F16", "", 0},
    {"F17", "", 0},
    {"F18", "", 0},
    {"F19", "", 0},
    {"F20", "", 0},
    {"F21", "", 0},
    {"F22", "", 0},
    {"F23", "", 0},
    {"F24", "", 0},
    {"F25", "", 0},
    {"F26", "", 0},
    {"F27", "", 0},
    {"F28", "", 0},
    {"F29", "", 0},
    {"F30", "", 0},
    {"F31", "", 0},
    {"Telex hinten", "couple", 0}, // Telexkupplung hinten
    {"Telex vorne", "couple", 0},  // Telexkupplung vorne
    {"Pantograf hinten", "", 0},
    {"Pantograf vorne", "", 0},
    {"Licht hinten", "", 0},
    {"Licht vorne", "", 0},
    {"Heben", "", 0},
    {"Lüfter", "", 0},
    {"Triebwerksbeleuchtung", "", 0},
    {"Zylinder ausblasen", "", 0},
    {"Dampfstoß", "", 0},
    {"Kran", "", 0},
    {"Auf", "", 1},
    {"Ab", "", 1},
    {"Links", "", 1},
    {"Rechts", "", 1},
    {"Drehen rechts", "", 1},
    {"Magnet", "", 0}};
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
        pos = getParameterHex(data, ".adresse=", locoData.adress);
        std::string adressType;
        pos = getParameter(data, ".typ=", adressType);
        if ("mm" == adressType || "mm2_prg" == adressType || "mm2_lok" == adressType || "mm2_dil8" == adressType)
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
                if (functionType.size() > fktType)
                {
                    locoData.functionData.emplace_back(FunctionData{functionType[fktType].iconName, functionType[fktType].shortcut, fktNum, functionType[fktType].buttonType, fktTime});
                }
                else
                {
                    locoData.functionData.emplace_back(FunctionData{"weight", "unknown", fktNum, 0, fktTime});
                }
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
            if ((0 != fktType) && (1 != fktType))
            {
                if (functionType.size() > fktType)
                {
                    locoData.functionData.emplace_back(FunctionData{functionType[fktType].iconName, functionType[fktType].shortcut, fktNum, functionType[fktType].buttonType, fktTime});
                }
                else
                {
                    locoData.functionData.emplace_back(FunctionData{"weight", "unknown", fktNum, 0, fktTime});
                }
            }
        }
        result = true;
    }
    return result;
};

std::string::size_type Cs2DataParser::getParameterHex(std::string *data, std::string parameter, uint16_t &result, std::string::size_type startpos)
{
    std::string dummy;
    std::string::size_type rv = getParameter(data, parameter, dummy, startpos);
    if (!dummy.empty())
    {
        result = static_cast<uint16_t>(std::stoul(dummy, nullptr, 16));
    }
    return rv;
}

std::string::size_type Cs2DataParser::getParameter(std::string *data, std::string parameter, uint16_t &result, std::string::size_type startpos)
{
    std::string dummy;
    std::string::size_type rv = getParameter(data, parameter, dummy, startpos);
    if (!dummy.empty())
    {
        result = static_cast<uint16_t>(std::stoul(dummy, nullptr, 10));
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