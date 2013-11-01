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
#include "cvfGeometryBuilder.h"
#include "cvfuWavefrontObjImport.h"

#include <iostream>
#include <fstream>
#include <sstream>

#ifdef WIN32
#pragma warning (disable: 4996)
#endif

namespace cvfu {

using cvf::ref;
using cvf::Vec3f;
using cvf::Vec2f;
using cvf::Vec3d;


//==================================================================================================
///
/// \class cvfu::WavefrontObjImport
/// \ingroup Utilities
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
WavefrontObjImport::WavefrontObjImport()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool WavefrontObjImport::readFile(const cvf::String& fileName)
{
    std::ifstream file;

#ifdef WIN32
    file.open(fileName.toAscii().ptr(), std::ios::in);
#else
    file.open(fileName.toStdString().c_str(), std::ios::in);
#endif

    if (!file.is_open())
    {
        return false;
    }


    std::string record = nextRecord(&file);

    int vertexCount = 0;

    while (!file.eof())
    {
        if (record.length() > 1)
        {
            if (record[0] == 'v' && record[1] == ' ')
            {
                // The v block contains vertex data
                // format:
                // v 0.1 0.2 0.3
                // v 1.2 3.1 3.5
                //
                // Need to check for space as second character, as the file can also contain normals with block 'vn' and 
                // texture coordinates with block 'vt'
                // 
                // No support for line continuation here, as this is never so far seen "in the wild"
                //
                char dummy;
                double x = cvf::UNDEFINED_DOUBLE;
                double y = cvf::UNDEFINED_DOUBLE;
                double z = cvf::UNDEFINED_DOUBLE;

                sscanf(record.c_str(), "%c %lf %lf %lf", &dummy, &x, &y, &z);

                if (x > cvf::UNDEFINED_DOUBLE_THRESHOLD || 
                    y > cvf::UNDEFINED_DOUBLE_THRESHOLD ||
                    z > cvf::UNDEFINED_DOUBLE_THRESHOLD)
                {
                    CVF_FAIL_MSG("Error reading file. Need error handling!");
                    return false;
                }

                // Add the node
                m_nodes.push_back(x);
                m_nodes.push_back(y);
                m_nodes.push_back(z);
                vertexCount++;
            }
            else if (record[0] == 'f' && record[1] == ' ')
            {
                // The f block contains face definitions
                // All values are ONE based indices into the given vertex, normal or texture arrays
                //
                //  Format:
                //  f v1 v2 v3 v4 ...
                //  f v1/vt1 v2/vt2 v3/vt3 ....
                //  f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3 ...
                //  f v1//vn1 v2//vn2 v3//vn3 ...
                //
                // Relative and absolute indices:
                // The format also supports relative indices, so specifying a negative index means counting backwards from the last index defined
                // -1 means the last index defined
                // 
                //  # example: 8 indices already defined
                //  f -2 -1 -4 -3
                //  # is equivalent to:
                //  f 7 8 5 6
                std::vector<std::string> elems;
                split(record, ' ', elems);

                m_faceList.push_back(static_cast<cvf::uint>(elems.size()) - 1);

                size_t i;
                for (i = 1; i < elems.size(); i++)
                {
                    int vertex = atoi(elems[i].c_str());

                    if (vertex > 0)
                    {
                        m_faceList.push_back(static_cast<cvf::uint>(vertex - 1));
                    }
                    else
                    {
                        // Note that vertex is negative, so this will subtract from vertexCount
                        // -1 is last index, so this will give zero based indices (file has one based)
                        int actualIndexZeroBased = vertexCount + vertex;
                        m_faceList.push_back(static_cast<cvf::uint>(actualIndexZeroBased));
                    }
                }
            }
        }

        record = nextRecord(&file);
    }

    // We now have a valid faceList and nodes. 
    // Update the IMesh with this data
    //mesh->setFromFaceList(faceList);
    //mesh->setNodes(nodes);

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string WavefrontObjImport::nextRecord(std::ifstream* stream)
{
    CVF_ASSERT(stream);

    std::string record;
    getline(*stream, record);

    if (record.length() > 0)
    {
        while (record[record.length() - 1] == '\\')
        {
            std::string line;
            getline(*stream, line);

            record[record.length() - 1] = ' ';
            record += line;
        }
    }

    return record;
}


//--------------------------------------------------------------------------------------------------
/// Helper to split a string into elements based on a limiter. To be moved 
//--------------------------------------------------------------------------------------------------
std::vector<std::string>& WavefrontObjImport::split(const std::string& s, char delim, std::vector<std::string>& elems)
{
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) 
    {
        if (item.length() > 0)
        {
            elems.push_back(item);
        }
    }

    return elems;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void WavefrontObjImport::buildGeometry(cvf::GeometryBuilder* builder)
{
    size_t numVertices = m_nodes.size()/3;
    cvf::Vec3fArray vertexArray;
    vertexArray.reserve(numVertices);
    size_t i;
    for (i = 0; i < numVertices; i++)
    {
        Vec3f v(static_cast<float>(m_nodes[3*i]), static_cast<float>(m_nodes[3*i + 1]), static_cast<float>(m_nodes[3*i + 2]));
        vertexArray.add(v);
    }

    builder->addVertices(vertexArray);

    size_t numFaceListEntries = m_faceList.size();
    cvf::UIntArray faceIndices;
    i = 0;
    while (i < numFaceListEntries)
    {
        cvf::uint numFaceVertices = m_faceList[i++];
        faceIndices.reserve(numFaceVertices);
        faceIndices.setSizeZero();
        size_t j;
        for (j = 0; j < numFaceVertices; j++)
        {
            faceIndices.add((m_faceList[i++]));
        }

        builder->addFace(faceIndices);
    }
}

} // namespace cvfu

