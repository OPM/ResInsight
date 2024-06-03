/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QDataStream>

namespace riOctavePlugin
{
const int qtDataStreamVersion = QDataStream::Qt_4_0;

// https://en.wikipedia.org/wiki/List_of_TCP_and_UDP_port_numbers
// Use a port number in the dynamic/private range (49152-65535)
const int defaultPortNumber = 52025;

inline const std::string portNumberKey()
{
    return "RESINSIGHT_OCTAVE_PORT_NUMBER";
}

} // namespace riOctavePlugin
