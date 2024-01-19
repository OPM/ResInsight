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

#include "RiaDefines.h"

#include "cvfObject.h"

#include <QStringList>
#include <vector>

class RimWellLogCurve;

//==================================================================================================
///
//==================================================================================================
class RigWellLogFile : public cvf::Object
{
public:
    RigWellLogFile();
    ~RigWellLogFile() override;

    virtual QString     date() const                = 0;
    virtual QString     wellName() const            = 0;
    virtual QStringList wellLogChannelNames() const = 0;

    virtual std::vector<double> depthValues() const  = 0;
    virtual std::vector<double> tvdMslValues() const = 0;
    virtual std::vector<double> tvdRkbValues() const = 0;

    virtual std::vector<double> values( const QString& name ) const = 0;

    virtual QString wellLogChannelUnitString( const QString& wellLogChannelName, RiaDefines::DepthUnitType displayDepthUnit ) const = 0;
    virtual QString wellLogChannelUnitString( const QString& wellLogChannelName ) const                                             = 0;
    virtual RiaDefines::DepthUnitType depthUnit() const                                                                             = 0;

    virtual bool hasTvdMslChannel() const = 0;
    virtual bool hasTvdRkbChannel() const = 0;

    virtual double getMissingValue() const = 0;
};
