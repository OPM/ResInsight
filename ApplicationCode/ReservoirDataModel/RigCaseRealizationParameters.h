/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include <QString>

#include <map>
#include <memory>


//==================================================================================================
//
//
//==================================================================================================
class RigCaseRealizationParameters : public cvf::Object
{
public:
    void                        addParameter(const QString& name, double value);
    double                      parameterValue(const QString& name);

    std::map<QString, double>   parameters() const;

private:
    std::map<QString, double>   m_parameters;
};
