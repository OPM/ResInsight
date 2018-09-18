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

#include <cstddef>
#include <vector>


//==================================================================================================
//
//
//
//==================================================================================================
class VdeArrayDataPacket
{
public:
    enum ElementType
    {
        Unknown = 0,
        Uint32,
        Float32
    };

public:
    ElementType             elementType() const;
    size_t                  elementSize() const;
    size_t                  elementCount() const;
    const char*             arrayData() const;

    size_t                  fullPacketSize() const;
    const char*             fullPacketRawPtr() const;

    static VdeArrayDataPacket  fromFloat32Arr(int packetId, const float* srcArr, size_t srcArrElementCount);
    static VdeArrayDataPacket  fromUint32Arr(int packetId, const unsigned int* srcArr, size_t srcArrElementCount);

    static VdeArrayDataPacket  fromRawPacketBuffer(const char* rawPacketBuffer, size_t bufferByteSize);

private:
    VdeArrayDataPacket();

    bool    assign(int packetId, ElementType elementType, size_t elementCount, const char* payloadPtr, size_t payloadSizeInBytes);

private:
    int                 m_packetId;
    ElementType         m_elementType;
    size_t              m_elementCount;

    std::vector<char>   m_dataBytes;
};

