/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RimWellLogChannel.h"

//==================================================================================================
///
///
//==================================================================================================
class RimOsduWellLogChannel : public RimWellLogChannel
{
    CAF_PDM_HEADER_INIT;

public:
    RimOsduWellLogChannel();

    void setId( const QString& id );
    void setDescription( const QString& description );
    void setTopDepth( double topDepth );
    void setBaseDepth( double baseDepth );
    void setInterpreterName( const QString& interpreterName );
    void setQuality( const QString& quality );
    void setUnit( const QString& unit );
    void setDepthUnit( const QString& depthUnit );

private:
    caf::PdmField<QString> m_id;
    caf::PdmField<QString> m_description;
    caf::PdmField<double>  m_topDepth;
    caf::PdmField<double>  m_baseDepth;
    caf::PdmField<QString> m_interpreterName;
    caf::PdmField<QString> m_quality;
    caf::PdmField<QString> m_unit;
    caf::PdmField<QString> m_depthUnit;
};
