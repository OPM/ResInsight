/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RigFemTypes.h"

#include <QString>

#include <cassert>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigFemTypes::elementNodeCount( RigElementType elmType )
{
    static int elementTypeCounts[3] = { 8, 8, 4 };

    return elementTypeCounts[(unsigned int)elmType];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigFemTypes::elementFaceCount( RigElementType elmType )
{
    const static int elementFaceCounts[3] = { 6, 6, 1 };

    return elementFaceCounts[(unsigned int)elmType];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// HEX8
//     7---------6     Increased k -> Increased depth : 4 5 6 7 is the deepest quad
//    /|        /|     |k
//   / |       / |     | /j
//  4---------5  |     |/
//  |  3------|--2     *---i
//  | /       | /
//  |/        |/
//  0---------1

const int* RigFemTypes::localElmNodeIndicesForFace( RigElementType elmType, int faceIdx, int* faceNodeCount )
{
    static const int HEX8_Faces[6][4] = { { 1, 2, 6, 5 }, { 0, 4, 7, 3 }, { 3, 7, 6, 2 }, { 0, 1, 5, 4 }, { 4, 5, 6, 7 }, { 0, 3, 2, 1 } };
    static const int CAX4_Faces[4]    = { 0, 1, 2, 3 };

    switch ( elmType )
    {
        case RigElementType::HEX8:
        case RigElementType::HEX8P:
            ( *faceNodeCount ) = 4;
            return HEX8_Faces[faceIdx];
            break;
        case RigElementType::CAX4:
            ( *faceNodeCount ) = 4;
            return CAX4_Faces;
            break;
        default:
            assert( false ); // Element type not supported
            break;
    }

    return CAX4_Faces;
}

int RigFemTypes::oppositeFace( RigElementType elmType, int faceIdx )
{
    static const int HEX8_OppositeFaces[6] = { 1, 0, 3, 2, 5, 4 };

    switch ( elmType )
    {
        case RigElementType::HEX8:
        case RigElementType::HEX8P:
            return HEX8_OppositeFaces[faceIdx];
            break;
        case RigElementType::CAX4:
            return faceIdx;
            break;
        default:
            assert( false ); // Element type not supported
            break;
    }

    return faceIdx;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

const int* RigFemTypes::localElmNodeToIntegrationPointMapping( RigElementType elmType )
{
    static const int HEX8_Mapping[8] = { 0, 1, 3, 2, 4, 5, 7, 6 };

    switch ( elmType )
    {
        case RigElementType::HEX8:
        case RigElementType::HEX8P:
            return HEX8_Mapping;
            break;
        case RigElementType::CAX4:
            return HEX8_Mapping; // First four is identical to HEX8
            break;
        default:
            assert( false ); // Element type not supported
            break;
    }

    return HEX8_Mapping;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RigFemTypes::elementTypeText( RigElementType elemType )
{
    std::string txt = "UNKNOWN_ELM_TYPE";

    switch ( elemType )
    {
        case RigElementType::HEX8:
            txt = "HEX8";
            break;
        case RigElementType::HEX8P:
            txt = "HEX8P";
            break;
        case RigElementType::CAX4:
            txt = "CAX4";
            break;
        case RigElementType::UNKNOWN_ELM_TYPE:
            break;
        default:
            break;
    }

    return txt;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<std::string, RigElementType> RigFemTypes::femTypeMap()
{
    std::map<std::string, RigElementType> typeMap;
    typeMap["C3D8R"]   = RigElementType::HEX8;
    typeMap["C3D8"]    = RigElementType::HEX8;
    typeMap["C3D8P"]   = RigElementType::HEX8P;
    typeMap["CAX4"]    = RigElementType::CAX4;
    typeMap["C3D20RT"] = RigElementType::HEX8;
    typeMap["C3D8RT"]  = RigElementType::HEX8;
    typeMap["C3D8T"]   = RigElementType::HEX8;

    return typeMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigElementType RigFemTypes::toRigElementType( const std::string odbTypeName )
{
    static std::map<std::string, RigElementType> odbElmTypeToRigElmTypeMap = femTypeMap();

    std::map<std::string, RigElementType>::iterator it = odbElmTypeToRigElmTypeMap.find( odbTypeName );

    if ( it == odbElmTypeToRigElmTypeMap.end() )
    {
        return RigElementType::UNKNOWN_ELM_TYPE;
    }

    return it->second;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemTypes::is8NodeElement( RigElementType elmType )
{
    return ( elmType == RigElementType::HEX8 ) || ( elmType == RigElementType::HEX8P );
}
