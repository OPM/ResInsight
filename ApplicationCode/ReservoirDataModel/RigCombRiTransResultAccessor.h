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

#include "RigResultAccessor.h"

#include "cvfCollection.h"

class RigGridBase;


//==================================================================================================
/// 
//==================================================================================================
class RigCombRiTransResultAccessor : public RigResultAccessor
{
public:
    RigCombRiTransResultAccessor(const RigGridBase* grid);

    void setPermResultAccessors(RigResultAccessor* xPermAccessor,
                                RigResultAccessor* yPermAccessor,
                                RigResultAccessor* zPermAccessor);
    void setNTGResultAccessor(  RigResultAccessor* ntgAccessor);

    /// CDARCY Darcy's constant 
    /// = 0.00852702 (E300); 0.008527 (ECLIPSE 100) (METRIC)
    /// = 0.00112712 (E300); 0.001127 (ECLIPSE 100) (FIELD)
    /// = 3.6 (LAB)
    /// = 0.00864 (PVT-M)
    void setCDARCHY(double cDarchy) { m_cdarchy = cDarchy;}

    virtual double  cellScalar(size_t gridLocalCellIndex) const;
    virtual double  cellFaceScalar(size_t gridLocalCellIndex, cvf::StructGridInterface::FaceType faceId) const;

private:

    cvf::ref<RigResultAccessor> m_xPermAccessor;
    cvf::ref<RigResultAccessor> m_yPermAccessor;
    cvf::ref<RigResultAccessor> m_zPermAccessor;

    cvf::ref<RigResultAccessor> m_ntgAccessor;

    double                      m_cdarchy;

    const RigGridBase*          m_grid;
};
