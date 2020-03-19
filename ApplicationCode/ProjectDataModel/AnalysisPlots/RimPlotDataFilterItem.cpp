/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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
#include "RimPlotDataFilterItem.h"

#include "RimAnalysisPlot.h"
#include "RimSummaryAddress.h"
#include "RimSummaryCaseCollection.h"
#include "cafPdmUiActionPushButtonEditor.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiPushButtonEditor.h"

namespace caf
{
template <>
void caf::AppEnum<RimPlotDataFilterItem::FilterTarget>::setUp()
{
    addItem( RimPlotDataFilterItem::SUMMARY_ITEM, "SUMMARY_ITEM", "Summary Item" );
    addItem( RimPlotDataFilterItem::SUMMARY_CASE, "SUMMARY_CASE", "Summary Case" );
    addItem( RimPlotDataFilterItem::ENSEMBLE_CASE, "ENSEMBLE_CASE", "Summary Case by Ensemble Parameter" );

    setDefault( RimPlotDataFilterItem::SUMMARY_CASE );
}

template <>
void caf::AppEnum<RimPlotDataFilterItem::FilterOperation>::setUp()
{
    addItem( RimPlotDataFilterItem::RANGE, "RANGE", "Range" );
    addItem( RimPlotDataFilterItem::TOP_N, "TOP_N", "Top N" );
    addItem( RimPlotDataFilterItem::MIN_N, "MIN_N", "Min N" );

    setDefault( RimPlotDataFilterItem::RANGE );
}

template <>
void caf::AppEnum<RimPlotDataFilterItem::TimeStepSourceType>::setUp()
{
    addItem( RimPlotDataFilterItem::PLOT_SOURCE_TIMESTEPS, "PLOT_SOURCE_TIMESTEPS", "Plot Source" );
    addItem( RimPlotDataFilterItem::LAST_TIMESTEP, "LAST_TIMESTEP", "Last" );
    addItem( RimPlotDataFilterItem::LAST_TIMESTEP_WITH_HISTORY, "LAST_TIMESTEP_WITH_HISTORY", "Last With History" );
    addItem( RimPlotDataFilterItem::FIRST_TIMESTEP, "FIRST_TIMESTEP", "First" );
    addItem( RimPlotDataFilterItem::ALL_TIMESTEPS, "ALL_TIMESTEPS", "All" );
    addItem( RimPlotDataFilterItem::SELECT_TIMESTEPS, "SELECT_TIMESTEPS", "By Selection" );
    addItem( RimPlotDataFilterItem::SELECT_TIMESTEP_RANGE, "SELECT_TIMESTEP_RANGE", "By Range" );

    setDefault( RimPlotDataFilterItem::PLOT_SOURCE_TIMESTEPS );
}

} // namespace caf

CAF_PDM_SOURCE_INIT( RimPlotDataFilterItem, "PlotDataFilterItem" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotDataFilterItem::RimPlotDataFilterItem()
{
    CAF_PDM_InitObject( "Plot Data Filter", ":/EnsembleCurveSet16x16.png", "", "" );

    CAF_PDM_InitField( &m_isActive, "IsActive", true, "Active", "", "", "" );
    m_isActive.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_filterTarget, "FilterTarget", "Filter Type", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_filterAddress, "FilterAddressField", "Filter Address", "", "", "" );
    m_filterAddress.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitField( &m_filterEnsembleParameter, "QuantityText", QString( "" ), "Quantity", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_filterQuantityUiField, "SelectedVariableDisplayVar", "Vector", "", "", "" );
    m_filterQuantityUiField.xmlCapability()->disableIO();
    m_filterQuantityUiField.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_filterQuantitySelectButton, "SelectAddress", false, "...", "", "", "" );
    caf::PdmUiActionPushButtonEditor::configureEditorForField( &m_filterQuantitySelectButton );

    CAF_PDM_InitFieldNoDefault( &m_filterOperation, "FilterOperation", "Operation", "", "", "" );
    CAF_PDM_InitField( &m_useAbsoluteValue, "UseAbsoluteValue", true, "Use Abs(value)", "", "", "" );
    CAF_PDM_InitField( &m_minTopN, "MinTopN", 20, "N", "", "", "" );
    CAF_PDM_InitField( &m_max, "Max", 0.0, "Max", "", "", "" );
    CAF_PDM_InitField( &m_min, "Min", 0.0, "Min", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_ensembleParameterValueCategories,
                                "EnsembleParameterValueCategories",
                                "Categories",
                                "",
                                "",
                                "" );
    CAF_PDM_InitFieldNoDefault( &m_consideredTimestepsType, "ConsideredTimestepsType", "Timesteps to Consider", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_explicitlySelectedTimeSteps, "ExplicitlySelectedTimeSteps", "TimeSteps", "", "", "" );
    m_explicitlySelectedTimeSteps.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotDataFilterItem::~RimPlotDataFilterItem()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimPlotDataFilterItem::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;
    RimAnalysisPlot*              parentPlot;
    this->firstAncestorOrThisOfTypeAsserted( parentPlot );

    if ( fieldNeedingOptions == &m_filterQuantityUiField )
    {
        if ( m_filterTarget != ENSEMBLE_CASE )
        {
            std::set<RifEclipseSummaryAddress> allAddresses = parentPlot->unfilteredAddresses();

            for ( auto& address : allAddresses )
            {
                if ( address.isErrorResult() ) continue;

                options.push_back( caf::PdmOptionItemInfo( QString::fromStdString( address.uiText() ),
                                                           QVariant::fromValue( address ) ) );
            }

            options.push_front( caf::PdmOptionItemInfo( RiaDefines::undefinedResultName(),
                                                        QVariant::fromValue( RifEclipseSummaryAddress() ) ) );
        }
    }
    else if ( fieldNeedingOptions == &m_filterEnsembleParameter )
    {
        // if ( m_filterTarget() == ENSEMBLE_CASE )
        // {
        //     std::set<EnsembleParameter> ensembleParams = parentPlot->ensembleParameters();
        //     for ( const EnsembleParameter& ensParam : ensembleParams )
        //     {
        //         options.push_back( caf::PdmOptionItemInfo( ensParam.uiName(), ensParam.name ) );
        //     }
        //
        //     options.push_front( caf::PdmOptionItemInfo( RiaDefines::undefinedResultName(),
        //                                                 QVariant::fromValue( RifEclipseSummaryAddress() ) ) );
        // }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimPlotDataFilterItem::objectToggleField()
{
    return &m_isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotDataFilterItem::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_filterTarget, {true, 5, 1} );
    if ( m_filterTarget() == ENSEMBLE_CASE )
    {
        uiOrdering.add( &m_filterEnsembleParameter, {true, 5, 1} );
    }
    else
    {
        uiOrdering.add( &m_filterQuantityUiField, {true, 5, 1} );
        // uiOrdering.add( &m_filterQuantitySelectButton, {false, 1, 0} );
    }

    uiOrdering.add( &m_filterOperation, {true, 3, 1} );
    uiOrdering.add( &m_useAbsoluteValue, {false} );

    if ( m_filterOperation() == RANGE )
    {
        uiOrdering.add( &m_max );
        uiOrdering.add( &m_min );
    }
    else if ( m_filterOperation == TOP_N || m_filterOperation == MIN_N )
    {
        uiOrdering.add( &m_minTopN );
    }

    if ( m_filterTarget() == ENSEMBLE_CASE && false ) // Ensemble Quantity is a category value
    {
        uiOrdering.add( &m_ensembleParameterValueCategories );
    }

    if ( m_filterTarget() != ENSEMBLE_CASE ) // Ensemble Quantity is a category value
    {
        uiOrdering.add( &m_consideredTimestepsType );
        if ( m_consideredTimestepsType == SELECT_TIMESTEPS || m_consideredTimestepsType == SELECT_TIMESTEP_RANGE )
        {
            uiOrdering.add( &m_explicitlySelectedTimeSteps );
        }
    }

    uiOrdering.skipRemainingFields( true );
}
