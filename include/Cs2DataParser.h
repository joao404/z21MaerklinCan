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

#pragma once

#include <Arduino.h>
#include <vector>
#include <string>

class Cs2DataParser
{
public:
	struct FunctionData
	{
		std::string imageName;
		std::string shortcut;
		uint16_t function;
		uint16_t buttonType;
		uint16_t time;
	};

	struct LocoData
	{
		std::string name;
		uint16_t adress;
		std::vector<FunctionData> functionData;
	};

	struct LocoFunctionType
	{
		const char* shortcut;
		const char* iconName;
		uint8_t buttonType;
	};

	static bool parseCs2ToLocoData(std::string *data, LocoData &locoData);
	static std::string::size_type getParameterHex(std::string *data, std::string parameter, uint16_t &result, std::string::size_type startpos = 0);
	static std::string::size_type getParameter(std::string *data, std::string parameter, uint16_t &result, std::string::size_type startpos = 0);
	static std::string::size_type getParameter(std::string *data, std::string parameter, std::string &result, std::string::size_type startpos = 0);
};