/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RimWellDiskConfig.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellDiskConfig::RimWellDiskConfig()
    : m_isSingleProperty( false )
    , m_summaryCase( nullptr )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellDiskConfig::~RimWellDiskConfig() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellDiskConfig::isSingleProperty() const
{
    return m_isSingleProperty;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimWellDiskConfig::getSingleProperty() const
{
    return m_singleProperty;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDiskConfig::setSingleProperty( const std::string& singleProperty )
{
    m_isSingleProperty = true;
    m_singleProperty   = singleProperty;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDiskConfig::setOilProperty( const std::string& oilProperty )
{
    m_isSingleProperty = false;
    m_oilProperty      = oilProperty;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimWellDiskConfig::getOilProperty() const
{
    return m_oilProperty;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDiskConfig::setGasProperty( const std::string& gasProperty )
{
    m_isSingleProperty = false;
    m_gasProperty      = gasProperty;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimWellDiskConfig::getGasProperty() const
{
    return m_gasProperty;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDiskConfig::setWaterProperty( const std::string& waterProperty )
{
    m_isSingleProperty = false;
    m_waterProperty    = waterProperty;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimWellDiskConfig::getWaterProperty() const
{
    return m_waterProperty;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimWellDiskConfig::sourceCase() const
{
    return m_summaryCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDiskConfig::setSourceCase( RimSummaryCase* summaryCase )
{
    m_summaryCase = summaryCase;
}
