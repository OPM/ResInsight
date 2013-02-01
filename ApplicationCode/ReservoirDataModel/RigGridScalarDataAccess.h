//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
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
//##################################################################################################

#pragma once

#include "cvfStructGridScalarDataAccess.h"
#include "RigGridBase.h"
#include "RifReaderInterface.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RigGridScalarDataAccess : public cvf::StructGridScalarDataAccess
{
public:
    RigGridScalarDataAccess(const RigGridBase* grid, RifReaderInterface::PorosityModelResultType porosityModel, size_t timeStepIndex, size_t scalarSetIndex);

    virtual double  cellScalar(size_t i, size_t j, size_t k) const;
    virtual double  cellScalar(size_t cellIndex) const;
    virtual void    cellCornerScalars(size_t i, size_t j, size_t k, double scalars[8]) const;
    virtual double  gridPointScalar(size_t i, size_t j, size_t k) const;
    virtual bool    pointScalar(const cvf::Vec3d& p, double* scalarValue) const;
    
    virtual const cvf::Vec3d* cellVector(size_t i, size_t j, size_t k) const;

private:
    cvf::cref<RigGridBase>  m_grid;
    bool                    m_useGlobalActiveIndex;
    std::vector<double>*    m_resultValues;
};

