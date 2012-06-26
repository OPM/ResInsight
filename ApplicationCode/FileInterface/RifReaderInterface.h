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

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfLibCore.h"

#include <QString>
#include <QStringList>


class RigReservoir;

//==================================================================================================
//
// Data interface base class
//
//==================================================================================================
class RifReaderInterface : public cvf::Object
{
public:
    RifReaderInterface() {}
    virtual ~RifReaderInterface() {}

    virtual bool                open(const QString& fileName, RigReservoir* reservoir) = 0;
    virtual void                close() = 0;
   
    virtual bool                staticResult(const QString& result, std::vector<double>* values) = 0;
    virtual bool                dynamicResult(const QString& result, size_t stepIndex, std::vector<double>* values) = 0;

};

