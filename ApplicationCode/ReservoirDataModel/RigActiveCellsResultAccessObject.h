/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA, Ceetron Solutions AS
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

#include "RigResultAccessObject.h"

class RigGridBase;
class RigActiveCellInfo;


//==================================================================================================
/// 
//==================================================================================================
class RigActiveCellsResultAccessObject : public RigResultAccessObject
{
public:
    RigActiveCellsResultAccessObject(const RigGridBase* grid, std::vector<double>* reservoirResultValues, const RigActiveCellInfo* activeCellInfo, const QString& resultName);

    virtual double  cellScalar(size_t localCellIndex) const;
    virtual double  cellFaceScalar(size_t localCellIndex, cvf::StructGridInterface::FaceType faceId) const;
    virtual QString resultName() const;
    virtual void    setCellScalar(size_t localCellIndex, double scalarValue);

private:
    const RigActiveCellInfo*    m_activeCellInfo;
    const RigGridBase*          m_grid;
    std::vector<double>*        m_reservoirResultValues;
    QString                     m_resultName;
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
