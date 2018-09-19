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
#include <string>


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
        Float32 = 1,
        Uint32  = 2,
        
    };

public:
    VdeArrayDataPacket();

    bool                        isValid() const;
    int                         arrayId() const;

    ElementType                 elementType() const;
    size_t                      elementSize() const;
    size_t                      elementCount() const;
    const char*                 arrayData() const;

    size_t                      fullPacketSize() const;
    const char*                 fullPacketRawPtr() const;

    static VdeArrayDataPacket   fromFloat32Arr(int arrayId, const float* srcArr, size_t srcArrElementCount);
    static VdeArrayDataPacket   fromUint32Arr(int arrayId, const unsigned int* srcArr, size_t srcArrElementCount);

    static VdeArrayDataPacket   fromRawPacketBuffer(const char* rawPacketBuffer, size_t bufferSize, std::string* errString);

private:
    bool    assign(int arrayId, ElementType elementType, size_t elementCount, const char* payloadPtr, size_t payloadSizeInBytes);

private:
    int                 m_arrayId;
    ElementType         m_elementType;
    size_t              m_elementCount;

    std::vector<char>   m_packetBytes;
};



//==================================================================================================
//
//
//
//==================================================================================================
class VdeBufferReader
{
public:
    VdeBufferReader(const char* buffer, size_t bufferSize);

    unsigned int    getUint32(size_t byteOffset) const;
    unsigned short  getUint16(size_t byteOffset) const;
    unsigned char   getUint8(size_t byteOffset) const;

private:
    const char*     m_buffer;
    const size_t    m_bufferSize;
};


//==================================================================================================
//
//
//
//==================================================================================================
class VdeBufferWriter
{
public:
    VdeBufferWriter(char* buffer, size_t bufferSize);

    void    setUint32(size_t byteOffset, unsigned int val);
    void    setUint16(size_t byteOffset, unsigned short val);
    void    setUint8(size_t byteOffset, unsigned char val);

private:
    char*           m_buffer;
    const size_t    m_bufferSize;
};

