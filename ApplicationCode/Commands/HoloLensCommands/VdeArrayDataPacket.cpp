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

// Binary package format/layout
//
//  packetVersion:          2 bytes
//  arrayId:                4 bytes     ID of this array
//  elementCount:           4 bytes     number of elements in array
//  elementType:            1 byte      data type of each element in the array(float32, uint32, uint8, int8)
//  imageComponentCount:    1 byte      number of image components for texture image(currently always 0 or 3)
//  imageWidth:             2 bytes     only used for texture images, otherwise 0
//  imageHeight:            2 bytes     :
//  arrayData:              ...


// Header offsets in bytes
static const size_t VDE_BYTEOFFSET_PACKET_VERSION       = 0;
static const size_t VDE_BYTEOFFSET_ARRAY_ID             = 2;
static const size_t VDE_BYTEOFFSET_ELEMENT_COUNT        = 6;
static const size_t VDE_BYTEOFFSET_ELEMENT_TYPE         = 10;
static const size_t VDE_BYTEOFFSET_IMAGE_COMPONENT_COUNT= 11;
static const size_t VDE_BYTEOFFSET_IMAGE_WIDTH          = 12;
static const size_t VDE_BYTEOFFSET_IMAGE_HEIGHT         = 14;

// Header size in bytes
static const size_t VDE_HEADER_SIZE = 2 + 4 + 4 + 1 + 1 + 2 + 2;

static const size_t VDE_PACKET_VERSION = 1;


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
VdeArrayDataPacket::VdeArrayDataPacket()
:   m_arrayId(-1),
    m_elementType(Unknown),
    m_elementCount(0)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool VdeArrayDataPacket::isValid() const
{
    if (m_elementType != Unknown && 
        m_packetBytes.size() >= VDE_HEADER_SIZE &&
        m_arrayId >= 0)
    {
        return true;
    }
    else
    {
        return false;
    }

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int VdeArrayDataPacket::arrayId() const
{
    return m_arrayId;
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
    if (m_packetBytes.size() > VDE_HEADER_SIZE)
    {
        const char* ptr = m_packetBytes.data();
        return ptr + VDE_HEADER_SIZE;
    }
    else
    {
        return nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
VdeArrayDataPacket VdeArrayDataPacket::fromFloat32Arr(int arrayId, const float* srcArr, size_t srcArrElementCount)
{
    size_t payloadByteCount = srcArrElementCount*sizeof(float);
    const char* rawSrcPtr = reinterpret_cast<const char*>(srcArr);

    VdeArrayDataPacket packet;
    packet.assign(arrayId, Float32, srcArrElementCount, rawSrcPtr, payloadByteCount);
    return packet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
VdeArrayDataPacket VdeArrayDataPacket::fromUint32Arr(int arrayId, const unsigned int* srcArr, size_t srcArrElementCount)
{
    size_t payloadByteCount = srcArrElementCount*sizeof(unsigned int);
    const char* rawSrcPtr = reinterpret_cast<const char*>(srcArr);

    VdeArrayDataPacket packet;
    packet.assign(arrayId, Uint32, srcArrElementCount, rawSrcPtr, payloadByteCount);
    return packet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
VdeArrayDataPacket VdeArrayDataPacket::fromRawPacketBuffer(const char* rawPacketBuffer, size_t bufferSize, std::string* errString)
{
    if (bufferSize < VDE_HEADER_SIZE)
    {
        if (errString) *errString = "Buffer size is less than fixed header size";
        return VdeArrayDataPacket();
    }

    VdeBufferReader bufferReader(rawPacketBuffer, bufferSize);
    const unsigned short packetVersion = bufferReader.getUint16(VDE_BYTEOFFSET_PACKET_VERSION);
    if (packetVersion != VDE_PACKET_VERSION)
    {
        if (errString) *errString = "Wrong packet version";
        return VdeArrayDataPacket();
    }

    const int packetId              = bufferReader.getUint32(VDE_BYTEOFFSET_ARRAY_ID);
    const ElementType elementType   = static_cast<ElementType>(bufferReader.getUint8(VDE_BYTEOFFSET_ELEMENT_TYPE));
    const size_t elementCount       = bufferReader.getUint32(VDE_BYTEOFFSET_ELEMENT_COUNT);

    const char* payloadPtr = rawPacketBuffer + VDE_HEADER_SIZE;
    const size_t payloadSizeInBytes = bufferSize - VDE_HEADER_SIZE;

    VdeArrayDataPacket packet;
    packet.assign(packetId, elementType, elementCount, payloadPtr, payloadSizeInBytes);

    return packet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool VdeArrayDataPacket::assign(int arrayId, ElementType elementType, size_t elementCount, const char* payloadPtr, size_t payloadSizeInBytes)
{
    const size_t totalSizeBytes = VDE_HEADER_SIZE + payloadSizeInBytes;
    m_packetBytes.resize(totalSizeBytes);

    VdeBufferWriter bufferWriter(m_packetBytes.data(), m_packetBytes.size());
    bufferWriter.setUint16(VDE_BYTEOFFSET_PACKET_VERSION,           VDE_PACKET_VERSION);
    bufferWriter.setUint32(VDE_BYTEOFFSET_ARRAY_ID,                 arrayId);
    bufferWriter.setUint32(VDE_BYTEOFFSET_ELEMENT_COUNT,            static_cast<unsigned int>(elementCount));
    bufferWriter.setUint8( VDE_BYTEOFFSET_ELEMENT_TYPE,             static_cast<unsigned char>(elementType));
    bufferWriter.setUint16(VDE_BYTEOFFSET_IMAGE_COMPONENT_COUNT,    0);
    bufferWriter.setUint16(VDE_BYTEOFFSET_IMAGE_WIDTH,              0);
    bufferWriter.setUint16(VDE_BYTEOFFSET_IMAGE_HEIGHT,             0);

    m_packetBytes.insert(m_packetBytes.begin() + VDE_HEADER_SIZE, payloadPtr, payloadPtr + payloadSizeInBytes);

    m_arrayId = arrayId;
    m_elementType = elementType;
    m_elementCount = elementCount;

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Size of the complete packet, including header, in bytes
//--------------------------------------------------------------------------------------------------
size_t VdeArrayDataPacket::fullPacketSize() const
{
    return m_packetBytes.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const char* VdeArrayDataPacket::fullPacketRawPtr() const
{
    return m_packetBytes.data();
}



//==================================================================================================
//
//
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
VdeBufferReader::VdeBufferReader(const char* buffer, size_t bufferSize)
:   m_buffer(buffer),
    m_bufferSize(bufferSize)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
unsigned int VdeBufferReader::getUint32(size_t byteOffset) const
{
    if (byteOffset + sizeof(unsigned int) <= m_bufferSize)
    {
        return *reinterpret_cast<const unsigned int*>(&m_buffer[byteOffset]);
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
unsigned short VdeBufferReader::getUint16(size_t byteOffset) const
{
    if (byteOffset + sizeof(unsigned short) <= m_bufferSize)
    {
        return *reinterpret_cast<const unsigned short*>(&m_buffer[byteOffset]);
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
unsigned char VdeBufferReader::getUint8(size_t byteOffset) const
{
    if (byteOffset + 1 <= m_bufferSize)
    {
        return *reinterpret_cast<const unsigned char*>(&m_buffer[byteOffset]);
    }

    return 0;
}



//==================================================================================================
//
//
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
VdeBufferWriter::VdeBufferWriter(char* buffer, size_t bufferSize)
:   m_buffer(buffer),
    m_bufferSize(bufferSize)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void VdeBufferWriter::setUint32(size_t byteOffset, unsigned int val)
{
    if (byteOffset + sizeof(unsigned int) <= m_bufferSize)
    {
        *reinterpret_cast<unsigned int*>(&m_buffer[byteOffset]) = val;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void VdeBufferWriter::setUint16(size_t byteOffset, unsigned short val)
{
    if (byteOffset + sizeof(unsigned short) <= m_bufferSize)
    {
        *reinterpret_cast<unsigned short*>(&m_buffer[byteOffset]) = val;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void VdeBufferWriter::setUint8(size_t byteOffset, unsigned char val)
{
    if (byteOffset + 1 <= m_bufferSize)
    {
        *reinterpret_cast<unsigned char*>(&m_buffer[byteOffset]) = val;
    }
}

