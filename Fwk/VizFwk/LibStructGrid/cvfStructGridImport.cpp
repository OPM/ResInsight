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
#include "cvfString.h"
#include "cvfTrace.h"
#include "cvfStructGridImport.h"

#include <stdio.h>

namespace cvf {



//==================================================================================================
///
/// \class cvf::StructGridImport
/// \ingroup StructGrid
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool StructGridImport::import(const cvf::String& filename, DataType dataType, uint countI, uint countJ, uint countK, cvf::DoubleArray* dataset)
{
    if (!dataset) return false;

    size_t numValues = countI * countJ * countK;
    dataset->resize(numValues);

#ifdef WIN32
    FILE* stream = NULL;
    errno_t err = fopen_s(&stream, filename.toUtf8().ptr(), "rb");
    if (err != 0 || !stream) return false;
#else
    FILE* stream = fopen(filename.toUtf8().ptr(), "rb");
    if (!stream) return false;
#endif


    Trace::show("Successfully opened " + filename);
    Trace::show("Starting to read %d", numValues);

    if (dataType == UBYTE)
    {
        UByteArray buffer;
        buffer.resize(numValues);

        size_t numRead = fread(buffer.ptr(), sizeof(ubyte), numValues, stream);
        if (numRead != numValues)
        {
            fclose(stream); 
            return false;
        }

        size_t i;
        for (i = 0; i < numValues; i++)
        {
            dataset->set(i, buffer[i]);
        }
    }
    else if (dataType == USHORT)
    {
        UShortArray buffer;
        buffer.resize(numValues);

        size_t numRead = fread(buffer.ptr(), sizeof(ushort), numValues, stream);
        if (numRead != numValues)
        {
            fclose(stream); 
            return false;
        }

        size_t i;
        for (i = 0; i < numValues; i++)
        {
            dataset->set(i, buffer[i]);
        }
    }

    fclose(stream); 

    Trace::show("Compleated reading data");

    return true;
}


} // namespace cvf

