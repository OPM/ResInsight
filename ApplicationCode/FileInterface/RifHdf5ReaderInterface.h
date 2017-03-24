/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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
class QDateTime;
class QStringList;

#include <vector>
#include <cstddef>

//==================================================================================================
// 
//
//==================================================================================================
class RifHdf5ReaderInterface
{
public:
    virtual std::vector<QDateTime>  timeSteps() const = 0;
    virtual QStringList             propertyNames() const = 0;
    virtual bool                    dynamicResult(const QString& result, size_t stepIndex, std::vector<double>* values) const = 0;
};
