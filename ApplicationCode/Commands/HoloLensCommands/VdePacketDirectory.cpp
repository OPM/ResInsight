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

#include "VdePacketDirectory.h"

#include <algorithm>

//==================================================================================================
//
//
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
VdePacketDirectory::VdePacketDirectory()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void VdePacketDirectory::addPacket( std::unique_ptr<VdeArrayDataPacket> packet )
{
    const int id        = packet->arrayId();
    m_idToPacketMap[id] = std::move( packet );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const VdeArrayDataPacket* VdePacketDirectory::lookupPacket( int arrayId ) const
{
    IdToPacketMap_T::const_iterator it = m_idToPacketMap.find( arrayId );
    if ( it == m_idToPacketMap.end() )
    {
        return nullptr;
    }

    return it->second.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void VdePacketDirectory::clear()
{
    m_idToPacketMap.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void VdePacketDirectory::pruneUnreferencedPackets( const std::vector<int>& packetIdsInUseArr )
{
    std::vector<int> sortedPacketsIdsInUse( packetIdsInUseArr );
    std::sort( sortedPacketsIdsInUse.begin(), sortedPacketsIdsInUse.end() );

    IdToPacketMap_T::const_iterator it = m_idToPacketMap.cbegin();
    while ( it != m_idToPacketMap.cend() )
    {
        const int packetId = it->first;
        if ( !std::binary_search( sortedPacketsIdsInUse.begin(), sortedPacketsIdsInUse.end(), packetId ) )
        {
            it = m_idToPacketMap.erase( it );
        }
        else
        {
            ++it;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool VdePacketDirectory::getPacketsAsCombinedBuffer( const std::vector<int>& packetIdsToGet,
                                                     QByteArray*             combinedPacketArr ) const
{
    for ( const int arrayId : packetIdsToGet )
    {
        IdToPacketMap_T::const_iterator it = m_idToPacketMap.find( arrayId );
        if ( it == m_idToPacketMap.end() )
        {
            return false;
        }

        const VdeArrayDataPacket& packet = *it->second;
        *combinedPacketArr +=
            QByteArray::fromRawData( packet.fullPacketRawPtr(), static_cast<int>( packet.fullPacketSize() ) );
    }

    return true;
}
