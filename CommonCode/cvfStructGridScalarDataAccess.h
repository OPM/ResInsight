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

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfVector3.h"


namespace cvf {


class StructGridScalarDataAccess : public Object
{
public:
    virtual double     cellScalar(size_t i, size_t j, size_t k) const = 0;
    virtual double     cellScalar(size_t cellIndex) const = 0;
    virtual void       cellCornerScalars(size_t i, size_t j, size_t k, double scalars[8]) const = 0;

    // Trenger vi denne? Kan erstattes av cellCornerScalars for kuttplan
    virtual double              gridPointScalar(size_t i, size_t j, size_t k) const = 0;
    virtual bool                pointScalar(const cvf::Vec3d& p, double* scalarValue) const = 0;

    // Vector results
    virtual const cvf::Vec3d*   cellVector(size_t i, size_t j, size_t k) const = 0;
};


} // namespace cvf
