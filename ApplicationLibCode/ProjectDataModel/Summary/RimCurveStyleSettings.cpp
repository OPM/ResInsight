/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RimCurveStyleSettings.h"

#include "RifEclipseSummaryAddress.h"

#include "RimCurveStyleRules.h"
#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"

namespace caf
{
template <>
void caf::AppEnum<RimCurveStyleSettings::Dimension>::setUp()
{
    using D = RimCurveStyleSettings::Dimension;
    addItem( D::NONE, "NONE", "None" );
    addItem( D::CASE, "CASE", "Summary Case" );
    addItem( D::VECTOR, "VECTOR", "Vector" );
    addItem( D::WELL, "WELL", "Well" );
    addItem( D::GROUP, "GROUP", "Group" );
    addItem( D::NETWORK, "NETWORK", "Network" );
    addItem( D::REGION, "REGION", "Region" );
    addItem( D::REGION_2_REGION, "REGION_2_REGION", "Region-Region" );
    addItem( D::AQUIFER, "AQUIFER", "Aquifer" );
    addItem( D::LGR, "LGR", "LGR" );
    addItem( D::BLOCK, "BLOCK", "Block" );
    addItem( D::CONNECTION, "CONNECTION", "Connection" );
    addItem( D::SEGMENT, "SEGMENT", "Segment" );
    setDefault( D::NONE );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimCurveStyleSettings, "RimCurveStyleSettings" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCurveStyleSettings::RimCurveStyleSettings()
{
    CAF_PDM_InitObject( "Global Curve Appearance" );

    CAF_PDM_InitField( &m_colorDimension, "ColorDimension", DimensionEnum( Dimension::CASE ), "Color", "", "Data dimension controlling the curve color" );
    CAF_PDM_InitField( &m_symbolDimension,
                       "SymbolDimension",
                       DimensionEnum( Dimension::NONE ),
                       "Symbol",
                       "",
                       "Data dimension controlling the curve symbol" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCurveStyleSettings::Dimension RimCurveStyleSettings::colorDimension() const
{
    return m_colorDimension();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCurveStyleSettings::Dimension RimCurveStyleSettings::symbolDimension() const
{
    return m_symbolDimension();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleSettings::setColorDimension( Dimension dimension )
{
    m_colorDimension = dimension;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleSettings::setSymbolDimension( Dimension dimension )
{
    m_symbolDimension = dimension;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimCurveStyleSettings::caseName( const RimSummaryCase* summaryCase )
{
    if ( !summaryCase ) return {};
    if ( summaryCase->ensemble() ) return summaryCase->ensemble()->name().toStdString();
    return summaryCase->displayCaseName().toStdString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimCurveStyleSettings::dimensionValue( Dimension dimension, const RifEclipseSummaryAddress& address, const std::string& caseName )
{
    using Category = RifEclipseSummaryAddressDefines::SummaryCategory;

    switch ( dimension )
    {
        case Dimension::CASE:
            return caseName;

        case Dimension::VECTOR:
        {
            std::string name = address.vectorName();
            if ( address.isHistoryVector() && !name.empty() ) name.pop_back();
            return name;
        }

        case Dimension::WELL:
            return address.wellName();

        case Dimension::GROUP:
            return address.groupName();

        case Dimension::NETWORK:
            return address.networkName();

        case Dimension::REGION:
            return address.category() == Category::SUMMARY_REGION ? address.itemUiText() : std::string();

        case Dimension::REGION_2_REGION:
            return address.category() == Category::SUMMARY_REGION_2_REGION ? address.itemUiText() : std::string();

        case Dimension::AQUIFER:
            return address.category() == Category::SUMMARY_AQUIFER ? address.itemUiText() : std::string();

        case Dimension::LGR:
            return address.lgrName();

        case Dimension::BLOCK:
            return address.category() == Category::SUMMARY_BLOCK || address.category() == Category::SUMMARY_BLOCK_LGR ? address.itemUiText()
                                                                                                                      : std::string();

        case Dimension::CONNECTION:
            return address.category() == Category::SUMMARY_WELL_COMPLETION || address.category() == Category::SUMMARY_WELL_CONNECTION ||
                           address.category() == Category::SUMMARY_WELL_CONNECTION_LGR
                       ? address.itemUiText()
                       : std::string();

        case Dimension::SEGMENT:
            return address.category() == Category::SUMMARY_WELL_SEGMENT ? address.itemUiText() : std::string();

        default:
            return std::string();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleSettings::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    // The collection list shows colors that depend on the global appearance
    if ( auto collection = firstAncestorOrThisOfType<RimCurveStyleRules>() )
    {
        collection->rulesChanged();
        collection->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleSettings::appendFieldsToUiOrdering( caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_colorDimension );
    uiOrdering.add( &m_symbolDimension );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleSettings::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    appendFieldsToUiOrdering( uiOrdering );
    uiOrdering.skipRemainingFields();
}
