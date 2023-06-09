/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023 Equinor ASA
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

#include "RimSeismicDataInterface.h"

#include "RimRegularLegendConfig.h"

CAF_PDM_XML_ABSTRACT_SOURCE_INIT( RimSeismicDataInterface, "SeismicDataInterface" ); // Abstract class.

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicDataInterface::RimSeismicDataInterface()
{
    CAF_PDM_InitObject( "SeismicDataBase" );

    CAF_PDM_InitFieldNoDefault( &m_legendConfig, "LegendDefinition", "Color Legend" );
    m_legendConfig = new RimRegularLegendConfig();
    m_legendConfig.uiCapability()->setUiTreeHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicDataInterface::~RimSeismicDataInterface()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSeismicDataInterface::gridIsEqual( RimSeismicDataInterface* other )
{
    if ( other == nullptr ) return false;

    bool equal = ( inlineMin() == other->inlineMin() ) && ( inlineMax() == other->inlineMax() ) && ( inlineStep() == other->inlineStep() );
    equal = equal && ( xlineMin() == other->xlineMin() ) && ( xlineMax() == other->xlineMax() ) && ( xlineStep() == other->xlineStep() );
    equal = equal && ( zMin() == other->zMin() ) && ( zMax() == other->zMax() ) && ( zStep() == other->zStep() );

    return equal;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig* RimSeismicDataInterface::legendConfig() const
{
    return m_legendConfig();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicDataInterface::initColorLegend()
{
    m_legendConfig->setColorLegend( RimRegularLegendConfig::mapToColorLegend( RimRegularLegendConfig::ColorRangesType::BLUE_WHITE_RED ) );
    m_legendConfig->setMappingMode( RimRegularLegendConfig::MappingType::LINEAR_CONTINUOUS );
    m_legendConfig->setRangeMode( RimLegendConfig::RangeModeType::USER_DEFINED );
    m_legendConfig->setCenterLegendAroundZero( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicAlphaMapper* RimSeismicDataInterface::alphaValueMapper() const
{
    return m_alphaValueMapper.get();
}
