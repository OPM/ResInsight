/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <QString>
#include <vector>

//==================================================================================================
/// PDM class for storing individual channel data (name and values) for well logs
//==================================================================================================
class RimWellLogChannelData : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellLogChannelData();

    void    setName( const QString& name );
    QString name() const;

    void                setValues( const std::vector<double>& values );
    std::vector<double> values() const;

private:
    caf::PdmField<QString>             m_name;
    caf::PdmField<std::vector<double>> m_values;
};