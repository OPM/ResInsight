/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "RimPlotAxisPropertiesInterface.h"

#include "RimSummaryMultiPlot.h"

#include "cafAppEnum.h"
#include "cafPdmUiTreeAttributes.h"

CAF_PDM_XML_ABSTRACT_SOURCE_INIT( RimPlotAxisPropertiesInterface,
                                  "PlotAxisPropertiesInterface",
                                  "RimPlotAxisPropertiesInterface" );

namespace caf
{
template <>
void caf::AppEnum<RimPlotAxisPropertiesInterface::AxisTitlePositionType>::setUp()
{
    addItem( RimPlotAxisPropertiesInterface::AXIS_TITLE_CENTER, "AXIS_TITLE_CENTER", "Center" );
    addItem( RimPlotAxisPropertiesInterface::AXIS_TITLE_END, "AXIS_TITLE_END", "At End" );

    setDefault( RimPlotAxisPropertiesInterface::AXIS_TITLE_CENTER );
}

template <>
void RimPlotAxisPropertiesInterface::LegendTickmarkCountEnum::setUp()
{
    addItem( RimPlotAxisPropertiesInterface::LegendTickmarkCount::TICKMARK_VERY_FEW, "VERY_FEW", "Very Few" );
    addItem( RimPlotAxisPropertiesInterface::LegendTickmarkCount::TICKMARK_FEW, "Few", "Few" );
    addItem( RimPlotAxisPropertiesInterface::LegendTickmarkCount::TICKMARK_DEFAULT, "Default", "Default" );
    addItem( RimPlotAxisPropertiesInterface::LegendTickmarkCount::TICKMARK_MANY, "Many", "Many" );
    setDefault( RimPlotAxisPropertiesInterface::LegendTickmarkCount::TICKMARK_DEFAULT );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotAxisPropertiesInterface::RimPlotAxisPropertiesInterface()
    : settingsChanged( this )
{
    CAF_PDM_InitObject( "Time Axis Interface" );

    CAF_PDM_InitField( &m_isAppearanceOverridden, "IsAppearanceOverridden", false, "IsAppearanceOverridden" );
    m_isAppearanceOverridden.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotAxisPropertiesInterface::isAxisInverted() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotAxisPropertiesInterface::isLogarithmicScaleEnabled() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisPropertiesInterface::setAppearanceOverridden( bool isOverridden )
{
    m_isAppearanceOverridden = isOverridden;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimPlotAxisPropertiesInterface::tickmarkCountFromEnum( LegendTickmarkCount count )
{
    int maxTickmarkCount = 8;

    switch ( count )
    {
        case LegendTickmarkCount::TICKMARK_VERY_FEW:
            maxTickmarkCount = 2;
            break;
        case LegendTickmarkCount::TICKMARK_FEW:
            maxTickmarkCount = 4;
            break;
        case LegendTickmarkCount::TICKMARK_DEFAULT:
            maxTickmarkCount = 8; // Taken from QwtPlot::initAxesData()
            break;
        case LegendTickmarkCount::TICKMARK_MANY:
            maxTickmarkCount = 10;
            break;
        default:
            break;
    }

    return maxTickmarkCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotAxisPropertiesInterface::isAppearanceOverridden() const
{
    return m_isAppearanceOverridden();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotAxisPropertiesInterface::defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    RimSummaryMultiPlot* summaryMultiPlot = nullptr;
    firstAncestorOfType( summaryMultiPlot );

    if ( summaryMultiPlot && summaryMultiPlot->isSubPlotAxesLinked() )
    {
        auto* treeItemAttribute = dynamic_cast<caf::PdmUiTreeViewItemAttribute*>( attribute );
        if ( treeItemAttribute )
        {
            treeItemAttribute->tags.clear();
            auto tag  = caf::PdmUiTreeViewItemAttribute::Tag::create();
            tag->icon = caf::IconProvider( ":/chain.png" );

            treeItemAttribute->tags.push_back( std::move( tag ) );
        }
    }
}
