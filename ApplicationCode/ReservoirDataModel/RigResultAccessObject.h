/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#pragma once

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfStructGrid.h"


class RigResultAccessObject : public cvf::Object
{
public:
    virtual double cellScalar(size_t localCellIndex) const = 0;
    virtual double cellFaceScalar(size_t localCellIndex, cvf::StructGridInterface::FaceType faceId) const = 0;

    virtual QString resultName() const = 0;

    virtual void   setCellScalar(size_t cellIndex, double value) = 0;
};


/*

class CellFaceScalarDataAccess : public Object
{
public:
    virtual double cellFaceScalar(size_t cellIndex, cvf::StructGridInterface faceId) const = 0;
};

class PlainCellFaceScalarDataAccess : public CellFaceScalarDataAccess
{
public:
    PlainCellFaceScalarDataAccess()
    {
        cellEdgeDataAccessObjects.resize(6);
    }

    void setDataAccessObjectForFace(cvf::StructGridInterface faceId, cvf::StructGridScalarDataAccess* dataAccessObject)
    {
        cellEdgeDataAccessObjects[faceId] = dataAccessObject;
    }

    virtual double cellFaceScalar( size_t cellIndex, cvf::StructGridInterface faceId ) const
    {
        cvf::StructGridScalarDataAccess* dataAccessObj = cellEdgeDataAccessObjects[faceId];
        if (dataAccessObj != NULL)
        {
            return dataAccessObj->cellScalar(cellIndex);
        }

        return HUGE_VAL;
    }
private:
    cvf::Collection<cvf::StructGridScalarDataAccess> cellEdgeDataAccessObjects;
};

*/
