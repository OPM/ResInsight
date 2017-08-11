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

#include "RifReaderInterface.h"

//==================================================================================================
//
// File interface for Eclipse output files
//
//==================================================================================================
class RifReaderEclipseInput : public RifReaderInterface
{
public:
    RifReaderEclipseInput();
    virtual ~RifReaderEclipseInput();

    // Virtual interface implementation
    virtual bool                open(const QString& fileName, RigEclipseCaseData* eclipseCase);

    

    virtual void                close() {}

    virtual bool                staticResult(const QString& result, RiaDefines::PorosityModelType matrixOrFracture, std::vector<double>* values )                      { return false; }
    virtual bool                dynamicResult(const QString& result, RiaDefines::PorosityModelType matrixOrFracture, size_t stepIndex, std::vector<double>* values )   { return false; }
};
