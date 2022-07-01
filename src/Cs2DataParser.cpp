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

bool Cs2DataParser::parseCs2ToLocoData(std::string *data, LocoData& locoData)
{
    bool result{false};
    Serial.println("check string");
    Serial.println(data->c_str());
    auto n = data->find("lokomotive\n");
    // text starts at beginning
    if(0 == n)
    {
        std::string::size_type pos = getParameter(data, ".name=", locoData.name);
        pos = getParameter(data, ".adresse=", locoData.adress, pos);
        //sscanf(data->c_str(), ".name=%s\n", locoData.name);
        // adress should by adapted by .typ=
        //sscanf(data->c_str(), ".adresse=%d\n", locoData.adress);
        locoData.functionData.emplace_back(FunctionData{"light", 2});
        result = true;
    }
    return result;
};

std::string::size_type Cs2DataParser::getParameter(std::string* data, std::string parameter, uint16_t& result, std::string::size_type startpos)
{
	std::string dummy;
	std::string::size_type rv = getParameter(data, parameter, dummy, startpos);
	result = static_cast<uint16_t>(std::stoul(dummy, nullptr, 16));
	return rv;
}

std::string::size_type Cs2DataParser::getParameter(std::string* data, std::string parameter, std::string& result, std::string::size_type startpos)
{
	std::string::size_type length = parameter.length();
	std::string::size_type start = data->find(parameter, startpos);
	if(std::string::npos != start)
	{
    std::string::size_type end = data->find('\n', start);
    result = data->substr(start + length, end - start - length);
    return end+1;
    }
    return std::string::npos;
}