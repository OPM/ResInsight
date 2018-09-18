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

#include "VdeArrayDataPacket.h"




//==================================================================================================
//
//
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
VdeArrayDataPacket::VdeArrayDataPacket()
:   m_packetId(-1),
    m_elementType(Unknown),
    m_elementCount(0)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool VdeArrayDataPacket::assign(int packetId, ElementType elementType, size_t elementCount, const char* payloadPtr, size_t payloadSizeInBytes)
{
    const size_t headerByteCount = 3*sizeof(int);
    const size_t totalSizeBytes = headerByteCount + payloadSizeInBytes;

    m_dataBytes.resize(totalSizeBytes);

    int* headerIntPtr = reinterpret_cast<int*>(m_dataBytes.data());
    headerIntPtr[0] = packetId;
    headerIntPtr[1] = elementType;
    headerIntPtr[2] = static_cast<int>(elementCount);

    m_dataBytes.insert(m_dataBytes.begin() + headerByteCount, payloadPtr, payloadPtr + payloadSizeInBytes);

    m_packetId = packetId;
    m_elementType = elementType;
    m_elementCount = elementCount;

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
VdeArrayDataPacket::ElementType VdeArrayDataPacket::elementType() const
{
    return m_elementType;
}

//--------------------------------------------------------------------------------------------------
/// Size of each element in bytes
//--------------------------------------------------------------------------------------------------
size_t VdeArrayDataPacket::elementSize() const
{
    switch (m_elementType)
    {
        case Uint32:    return sizeof(unsigned int);
        case Float32:   return sizeof(float);
        case Unknown:   return 0;
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
/// Return number of elements in the array
//--------------------------------------------------------------------------------------------------
size_t VdeArrayDataPacket::elementCount() const
{
    return m_elementCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const char* VdeArrayDataPacket::arrayData() const
{
    const size_t headerByteCount = 3*sizeof(int);
    const char* ptr = m_dataBytes.data();
    return ptr + headerByteCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
VdeArrayDataPacket VdeArrayDataPacket::fromFloat32Arr(int packetId, const float* srcArr, size_t srcArrElementCount)
{
    size_t payloadByteCount = srcArrElementCount*sizeof(float);
    const char* rawSrcPtr = reinterpret_cast<const char*>(srcArr);

    VdeArrayDataPacket packet;
    packet.assign(packetId, Float32, srcArrElementCount, rawSrcPtr, payloadByteCount);
    return packet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
VdeArrayDataPacket VdeArrayDataPacket::fromUint32Arr(int packetId, const unsigned int* srcArr, size_t srcArrElementCount)
{
    size_t payloadByteCount = srcArrElementCount*sizeof(unsigned int);
    const char* rawSrcPtr = reinterpret_cast<const char*>(srcArr);

    VdeArrayDataPacket packet;
    packet.assign(packetId, Uint32, srcArrElementCount, rawSrcPtr, payloadByteCount);
    return packet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
VdeArrayDataPacket VdeArrayDataPacket::fromRawPacketBuffer(const char* rawPacketBuffer, size_t bufferByteSize)
{
    const size_t headerByteCount = 3*sizeof(int);
    if (bufferByteSize < headerByteCount)
    {
        return VdeArrayDataPacket();
    }

    const int* headerIntPtr = reinterpret_cast<const int*>(rawPacketBuffer);
    const int packetId = headerIntPtr[0];
    const ElementType elementType = static_cast<ElementType>(headerIntPtr[1]);
    const size_t elementCount = headerIntPtr[2];

    const char* payloadPtr = rawPacketBuffer + headerByteCount;
    const size_t payloadSizeInBytes = bufferByteSize - headerByteCount;

    VdeArrayDataPacket packet;
    packet.assign(packetId, elementType, elementCount, payloadPtr, payloadSizeInBytes);
    return packet;
}

//--------------------------------------------------------------------------------------------------
/// Size of the complete packet, including header, in bytes
//--------------------------------------------------------------------------------------------------
size_t VdeArrayDataPacket::fullPacketSize() const
{
    return m_dataBytes.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const char* VdeArrayDataPacket::fullPacketRawPtr() const
{
    return m_dataBytes.data();
}

