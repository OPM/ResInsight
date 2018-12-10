/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018     Statoil ASA
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

#include "VdeArrayDataPacket.h"

#include <QByteArray>

#include <map>
#include <memory>


//==================================================================================================
//
//
//
//==================================================================================================
class VdePacketDirectory
{
public:
    VdePacketDirectory();

    void                        addPacket(const VdeArrayDataPacket& packet);
    const VdeArrayDataPacket*   lookupPacket(int arrayId) const;
    void                        clear();

    bool                        getPacketsAsCombinedBuffer(const std::vector<int>& packetIdsToGet, QByteArray* combinedPacketArr) const;

private:
    typedef std::map<int, std::unique_ptr<VdeArrayDataPacket>>  IdToPacketMap_T;
    
    IdToPacketMap_T   m_idToPacketMap;
};

