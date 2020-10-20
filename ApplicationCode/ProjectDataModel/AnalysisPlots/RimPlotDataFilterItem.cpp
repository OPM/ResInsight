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

#include "RifSummaryReaderInterface.h"
#include "RimSummaryCase.h"

#include "QFontMetrics"
#include "cafPdmUiActionPushButtonEditor.h"
#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiPushButtonEditor.h"

#include <limits>

namespace caf
{
template <>
void caf::AppEnum<RimPlotDataFilterItem::FilterTarget>::setUp()
{
    addItem( RimPlotDataFilterItem::SUMMARY_ITEM, "SUMMARY_ITEM", "summary items" );
    addItem( RimPlotDataFilterItem::SUMMARY_CASE, "SUMMARY_CASE", "summary cases" );
    addItem( RimPlotDataFilterItem::ENSEMBLE_CASE, "ENSEMBLE_CASE", "ensemble cases" );

    setDefault( RimPlotDataFilterItem::ENSEMBLE_CASE );
}

template <>
void caf::AppEnum<RimPlotDataFilterItem::FilterOperation>::setUp()
{
    addItem( RimPlotDataFilterItem::RANGE, "RANGE", "within range" );
    addItem( RimPlotDataFilterItem::TOP_N, "TOP_N", "top" );
    addItem( RimPlotDataFilterItem::BOTTOM_N, "BOTTOM_N", "bottom" );

    setDefault( RimPlotDataFilterItem::RANGE );
}

template <>
void caf::AppEnum<RimPlotDataFilterItem::TimeStepSourceType>::setUp()
{
    addItem( RimPlotDataFilterItem::PLOT_SOURCE_TIMESTEPS, "PLOT_SOURCE_TIMESTEPS", "plot source timesteps" );
    addItem( RimPlotDataFilterItem::LAST_TIMESTEP, "LAST_TIMESTEP", "last timestep" );
    addItem( RimPlotDataFilterItem::LAST_TIMESTEP_WITH_HISTORY, "LAST_TIMESTEP_WITH_HISTORY", "last timestep with history" );
    addItem( RimPlotDataFilterItem::FIRST_TIMESTEP, "FIRST_TIMESTEP", "first timestep" );
    addItem( RimPlotDataFilterItem::ALL_TIMESTEPS, "ALL_TIMESTEPS", "all timesteps" );
    addItem( RimPlotDataFilterItem::SELECT_TIMESTEPS, "SELECT_TIMESTEPS", "selected timesteps" );
    addItem( RimPlotDataFilterItem::SELECT_TIMESTEP_RANGE, "SELECT_TIMESTEP_RANGE", "timestep range" );

    setDefault( RimPlotDataFilterItem::PLOT_SOURCE_TIMESTEPS );
}

} // namespace caf

CAF_PDM_SOURCE_INIT( RimPlotDataFilterItem, "PlotDataFilterItem" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotDataFilterItem::RimPlotDataFilterItem()
    : m_lowerLimit( -std::numeric_limits<double>::infinity() )
    , m_upperLimit( std::numeric_limits<double>::infinity() )
    , filterChanged( this )
{
    CAF_PDM_InitObject( "Plot Data Filter", ":/AnalysisPlotFilter16x16.png", "", "" );

    CAF_PDM_InitField( &m_isActive, "IsActive", true, "Active", "", "", "" );
    m_isActive.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_filterTarget, "FilterTarget", "Use only the", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_filterAddress, "FilterAddressField", "Filter Address", "", "", "" );
    m_filterAddress.uiCapability()->setUiTreeHidden( true );
    m_filterAddress.uiCapability()->setUiTreeChildrenHidden( true );
    m_filterAddress = new RimSummaryAddress();

    CAF_PDM_InitField( &m_filterEnsembleParameter, "QuantityText", QString( "" ), "where", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_filterQuantityUiField, "SelectedVariableDisplayVar", "where", "", "", "" );
    m_filterQuantityUiField.xmlCapability()->disableIO();
    m_filterQuantityUiField.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_filterQuantitySelectButton, "SelectAddress", false, "...", "", "", "" );
    caf::PdmUiActionPushButtonEditor::configureEditorForField( &m_filterQuantitySelectButton );

    CAF_PDM_InitFieldNoDefault( &m_filterOperation, "FilterOperation", "is", "", "", "" );
    CAF_PDM_InitField( &m_topBottomN, "MinTopN", 20, "N", "", "", "" );
    m_topBottomN.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitField( &m_max, "Max", m_upperLimit, "Max", "", "", "" );
    m_max.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_min, "Min", m_lowerLimit, "Min", "", "", "" );
    m_min.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_ensembleParameterValueCategories, "EnsembleParameterValueCategories", "one of", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_consideredTimestepsType, "ConsideredTimestepsType", "at the", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_explicitlySelectedTimeSteps, "ExplicitlySelectedTimeSteps", "TimeSteps", "", "", "" );
    m_explicitlySelectedTimeSteps.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );
    m_explicitlySelectedTimeSteps.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    setDeletable( true );
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
bool RimPlotDataFilterItem::isValid() const
{
    if ( m_filterTarget() == ENSEMBLE_CASE && ensembleParameterName().isEmpty() )
    {
        return false;
    }
    else if ( ( m_filterTarget() == SUMMARY_CASE || m_filterTarget() == SUMMARY_ITEM ) && !summaryAddress().isValid() )
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RimPlotDataFilterItem::summaryAddress() const
{
    CVF_ASSERT( m_filterTarget() == SUMMARY_CASE || m_filterTarget() == SUMMARY_ITEM );
    return m_filterAddress->address();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimPlotDataFilterItem::ensembleParameterName() const
{
    CVF_ASSERT( m_filterTarget() == ENSEMBLE_CASE );
    return m_filterEnsembleParameter();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RimPlotDataFilterItem::filterRangeMinMax() const
{
    CVF_ASSERT( m_filterOperation() == RANGE );
    return std::make_pair( m_min(), m_max() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimPlotDataFilterItem::topBottomN() const
{
    CVF_ASSERT( m_filterOperation() != RANGE );
    return m_topBottomN;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimPlotDataFilterItem::selectedEnsembleParameterCategories() const
{
    CVF_ASSERT( m_filterTarget() == ENSEMBLE_CASE );
    return m_ensembleParameterValueCategories;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotDataFilterItem::TimeStepSourceType RimPlotDataFilterItem::consideredTimeStepsType() const
{
    CVF_ASSERT( m_filterTarget() != ENSEMBLE_CASE );
    return m_consideredTimestepsType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<time_t, time_t> RimPlotDataFilterItem::timeRangeMinMax() const
{
    CVF_ASSERT( m_consideredTimestepsType() == SELECT_TIMESTEP_RANGE );

    if ( m_explicitlySelectedTimeSteps().size() >= 2 )
    {
        time_t minTime = m_explicitlySelectedTimeSteps().front().toTime_t();
        time_t maxTime = m_explicitlySelectedTimeSteps().back().toTime_t();

        return std::make_pair( minTime, maxTime );
    }
    return std::make_pair( time_t( 0 ), time_t( 0 ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RimPlotDataFilterItem::explicitlySelectedTimeSteps() const
{
    CVF_ASSERT( m_consideredTimestepsType == RimPlotDataFilterItem::SELECT_TIMESTEPS );

    std::vector<time_t> selectedTimesteps;
    {
        for ( const QDateTime& dateTime : m_explicitlySelectedTimeSteps() )
        {
            selectedTimesteps.push_back( dateTime.toTime_t() );
        }
    }

    return selectedTimesteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotDataFilterItem::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                              const QVariant&            oldValue,
                                              const QVariant&            newValue )
{
    if ( changedField == &m_filterTarget )
    {
        this->updateMaxMinAndDefaultValues( true );
    }
    else if ( changedField == &m_filterQuantityUiField )
    {
        m_filterAddress->setAddress( m_filterQuantityUiField );
        this->updateMaxMinAndDefaultValues( true );
    }
    else if ( changedField == &m_filterEnsembleParameter )
    {
        this->updateMaxMinAndDefaultValues( true );
    }
    else if ( changedField == &m_filterOperation )
    {
        this->updateMaxMinAndDefaultValues( false );
    }
    else if ( changedField == &m_consideredTimestepsType || changedField == &m_explicitlySelectedTimeSteps )
    {
        this->updateMaxMinAndDefaultValues( false );
    }
    filterChanged.send();
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
        if ( m_filterTarget() == ENSEMBLE_CASE )
        {
            std::set<EnsembleParameter> ensembleParams = parentPlot->ensembleParameters();
            for ( const EnsembleParameter& ensParam : ensembleParams )
            {
                options.push_back( caf::PdmOptionItemInfo( ensParam.uiName(), ensParam.name ) );
            }
        }
    }
    else if ( fieldNeedingOptions == &m_ensembleParameterValueCategories )
    {
        EnsembleParameter eParm = selectedEnsembleParameter();
        if ( eParm.isText() )
        {
            for ( const auto& val : eParm.values )
            {
                options.push_back( caf::PdmOptionItemInfo( val.toString(), val.toString() ) );
            }
        }
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
    if ( m_filterAddress )
    {
        m_filterQuantityUiField = m_filterAddress->address();
    }

    updateMaxMinAndDefaultValues( false );

    uiOrdering.add( &m_filterTarget, {true, -1, 1} );
    if ( m_filterTarget() == ENSEMBLE_CASE )
    {
        uiOrdering.add( &m_filterEnsembleParameter, {true, caf::PdmUiOrdering::LayoutOptions::MAX_COLUMN_SPAN, 1} );
    }
    else
    {
        uiOrdering.add( &m_filterQuantityUiField, {true, caf::PdmUiOrdering::LayoutOptions::MAX_COLUMN_SPAN, 1} );
        // uiOrdering.add( &m_filterQuantitySelectButton, {false, 1, 0} );
    }
    if ( m_filterTarget() != ENSEMBLE_CASE )
    {
        uiOrdering.add( &m_consideredTimestepsType, {true, caf::PdmUiOrdering::LayoutOptions::MAX_COLUMN_SPAN, 1} );
        if ( m_consideredTimestepsType == SELECT_TIMESTEPS || m_consideredTimestepsType == SELECT_TIMESTEP_RANGE )
        {
            uiOrdering.add( &m_explicitlySelectedTimeSteps );
        }
    }

    EnsembleParameter eParm;
    if ( m_filterTarget() == ENSEMBLE_CASE )
    {
        eParm = selectedEnsembleParameter();
    }

    if ( m_filterTarget() == ENSEMBLE_CASE && eParm.isText() ) // Ensemble Quantity is a category value
    {
        uiOrdering.add( &m_ensembleParameterValueCategories );
    }
    else
    {
        uiOrdering.add( &m_filterOperation, {true, 2, 1} );

        if ( m_filterOperation() == RANGE )
        {
            uiOrdering.add( &m_max, {true, caf::PdmUiOrdering::LayoutOptions::MAX_COLUMN_SPAN, 1} );
            uiOrdering.add( &m_min, {true, caf::PdmUiOrdering::LayoutOptions::MAX_COLUMN_SPAN, 1} );
        }
        else if ( m_filterOperation == TOP_N || m_filterOperation == BOTTOM_N )
        {
            uiOrdering.add( &m_topBottomN, {false} );
        }
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotDataFilterItem::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                   QString                    uiConfigName,
                                                   caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_min || field == &m_max )
    {
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute );
        if ( !myAttr )
        {
            return;
        }

        myAttr->m_minimum                       = m_lowerLimit;
        myAttr->m_maximum                       = m_upperLimit;
        myAttr->m_delaySliderUpdateUntilRelease = true;
    }
    else if ( field == &m_topBottomN )
    {
        caf::PdmUiLineEditorAttribute* myAttr = dynamic_cast<caf::PdmUiLineEditorAttribute*>( attribute );

        if ( !myAttr ) return;

        QFontMetrics fm = QFontMetrics( QFont() );

        myAttr->maximumWidth = fm.boundingRect( "XXXX" ).width();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotDataFilterItem::updateMaxMinAndDefaultValues( bool forceDefault )
{
    RimAnalysisPlot* parentPlot;
    this->firstAncestorOrThisOfTypeAsserted( parentPlot );

    if ( m_filterTarget == ENSEMBLE_CASE )
    {
        if ( !selectedEnsembleParameter().isValid() )
        {
            std::set<EnsembleParameter> ensembleParams = parentPlot->ensembleParameters();
            if ( !ensembleParams.empty() )
            {
                m_filterEnsembleParameter = ensembleParams.begin()->name;
            }
        }

        EnsembleParameter eParam = selectedEnsembleParameter();
        if ( eParam.isValid() && eParam.isNumeric() )
        {
            if ( RiaCurveDataTools::isValidValue( eParam.minValue, false ) )
            {
                m_lowerLimit = eParam.minValue;
            }
            if ( RiaCurveDataTools::isValidValue( eParam.maxValue, false ) )
            {
                m_upperLimit = eParam.maxValue;
            }

            if ( m_upperLimit < m_lowerLimit )
            {
                std::swap( m_upperLimit, m_lowerLimit );
            }
        }
    }
    else
    {
        parentPlot->maxMinValueFromAddress( m_filterQuantityUiField,
                                            m_consideredTimestepsType(),
                                            m_explicitlySelectedTimeSteps(),
                                            false,
                                            &m_lowerLimit,
                                            &m_upperLimit );
    }

    if ( forceDefault || !( m_min >= m_lowerLimit && m_min <= m_upperLimit ) ) m_min = m_lowerLimit;
    if ( forceDefault || !( m_max >= m_lowerLimit && m_max <= m_upperLimit ) ) m_max = m_upperLimit;

    m_min.uiCapability()->setUiName( QString( "Min (%1)" ).arg( m_lowerLimit ) );
    m_max.uiCapability()->setUiName( QString( "Max (%1)" ).arg( m_upperLimit ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
EnsembleParameter RimPlotDataFilterItem::selectedEnsembleParameter() const
{
    RimAnalysisPlot* parentPlot;
    this->firstAncestorOrThisOfTypeAsserted( parentPlot );
    return parentPlot->ensembleParameter( m_filterEnsembleParameter );
}
