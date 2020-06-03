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

#pragma once

class QString;

enum RigElementType
{
    HEX8,
    HEX8P,
    CAX4,
    UNKNOWN_ELM_TYPE
};

class RigFemTypes
{
public:
    static int        elementNodeCount( RigElementType elmType );
    static int        elementFaceCount( RigElementType elmType );
    static const int* localElmNodeIndicesForFace( RigElementType elmType, int faceIdx, int* faceNodeCount );
    static int        oppositeFace( RigElementType elmType, int faceIdx );
    static const int* localElmNodeToIntegrationPointMapping( RigElementType elmType );
    static QString    elementTypeText( RigElementType elemType );
};
