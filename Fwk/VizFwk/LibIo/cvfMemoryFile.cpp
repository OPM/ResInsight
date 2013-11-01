//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################


#include "cvfBase.h"
#include "cvfMemoryFile.h"

#include <cstdio>


namespace cvf {

//==================================================================================================
///
/// \class cvf::MemoryFile
/// \ingroup Io
///
/// File read into memory
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
MemoryFile::MemoryFile()
{

}


//--------------------------------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------------------------------
MemoryFile::~MemoryFile()
{
    unload();
}


//--------------------------------------------------------------------------------------------------
/// Load file into memory. Old data will be destroyed.
/// 
/// \todo  Revisit when we have error objects in production to provide fine grain feedback on error.
//--------------------------------------------------------------------------------------------------
bool MemoryFile::load(const cvf::String& filename)
{
    unload();

    if (filename.isEmpty()) return false;

#ifdef WIN32
    // Open File;
    FILE* file = NULL;
    errno_t err = _wfopen_s(&file, filename.c_str(), L"rb");
    if (err != 0)
    {
        //CVF_ERROR(String("Unable to open file [%1]. : Error [%1]").arg(filename).arg((int)err));
        return false;
    }
#else
    // Open File;
    FILE* file = fopen(filename.toUtf8().ptr(), "rb");
    if (!file)
    {
        //CVF_ERROR(String("Unable to open file [%1].").arg(filename));
        return false;
    }
#endif

    // Find file size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    if (fileSize <= 0)
    {
        // No data, return as an error
        fclose(file);
        //CVF_ERROR(String("Unable to read file [%1]. It's empty.").arg(filename));
        return false;
    }

    // Safe to cast to unsigned type
    size_t bufferSize = static_cast<size_t>(fileSize) * sizeof(ubyte);

    // Read data from file into memory
    m_filename = filename;
    m_data.resize(bufferSize);
    fseek(file, 0, SEEK_SET);
    size_t bytesRead = fread(m_data.ptr(), 1, bufferSize, file);
    fclose(file);

    if (bytesRead != bufferSize)
    {
        //CVF_ERROR(String("Unable to to read file [%1]\n\tFilesize: %2, Bytes read: %3").arg(m_filename).arg((uint)bufferSize).arg((uint)bytesRead));
        return false;
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// Destroy old data
//--------------------------------------------------------------------------------------------------
void MemoryFile::unload()
{
    m_filename = L"";
    m_data.clear();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool MemoryFile::isEmpty() const
{
    return m_data.size() == 0 ? true : false;
}


//--------------------------------------------------------------------------------------------------
/// Get pointer to the currently loaded file data, if any. NULL if not set.
//--------------------------------------------------------------------------------------------------
const UByteArray* MemoryFile::data() const
{
    return &m_data;
}


//--------------------------------------------------------------------------------------------------
/// Get name of the currently loaded file. An empty string if not set.
//--------------------------------------------------------------------------------------------------
const String& MemoryFile::filename() const
{
    return m_filename;
}

}  // namespace cvf
