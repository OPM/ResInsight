/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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

#include "RimEnsembleCurveFilter.h"

#include "RiaCurveDataTools.h"

#include "RimCustomObjectiveFunction.h"
#include "RimCustomObjectiveFunctionCollection.h"
#include "RimEnsembleCurveFilterCollection.h"
#include "RimEnsembleCurveSet.h"
#include "RimSummaryAddress.h"
#include "RimSummaryCase.h"

#include "RiuSummaryVectorSelectionDialog.h"

#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiPushButtonEditor.h"

#include <algorithm>

namespace caf
{
template <>
void caf::AppEnum<RimEnsembleCurveFilter::FilterMode>::setUp()
{
    addItem( RimEnsembleCurveFilter::FilterMode::BY_ENSEMBLE_PARAMETER, "BY_ENSEMBLE_PARAMETER", "By Ensemble Parameter" );
    addItem( RimEnsembleCurveFilter::FilterMode::BY_OBJECTIVE_FUNCTION, "BY_OBJECTIVE_FUNCTION", "By Objective Function" );
    addItem( RimEnsembleCurveFilter::FilterMode::BY_CUSTOM_OBJECTIVE_FUNCTION,
             "By_CUSTOM_OBJECTIVE_FUNCTION",
             "By Custom Objective Function" );
    setDefault( RimEnsembleCurveFilter::FilterMode::BY_ENSEMBLE_PARAMETER );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimEnsembleCurveFilter, "RimEnsembleCurveFilter" );

//--------------------------------------------------------------------------------------------------
/// Internal constants
//--------------------------------------------------------------------------------------------------
#define DOUBLE_INF std::numeric_limits<double>::infinity()

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveFilter::RimEnsembleCurveFilter()
    : m_lowerLimit( -DOUBLE_INF )
    , m_upperLimit( DOUBLE_INF )
{
    CAF_PDM_InitObject( "Ensemble Curve Filter", ":/Filter.svg", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_filterTitle, "FilterTitle", "Title", "", "", "" );
    m_filterTitle.registerGetMethod( this, &RimEnsembleCurveFilter::description );
    m_filterTitle.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_active, "Active", "Active", "", "", "" );
    m_active = true;

    CAF_PDM_InitFieldNoDefault( &m_filterMode, "FilterMode", "Filter Mode", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_ensembleParameterName, "EnsembleParameter", "Ensemble Parameter", "", "", "" );
    m_ensembleParameterName.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_objectiveValuesSummaryAddressesUiField, "SelectedObjectiveSummaryVar", "Vector", "", "", "" );
    m_objectiveValuesSummaryAddressesUiField.xmlCapability()->disableIO();
    m_objectiveValuesSummaryAddressesUiField.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_objectiveValuesSummaryAddresses, "ObjectiveSummaryAddress", "Summary Address", "", "", "" );
    m_objectiveValuesSummaryAddresses.uiCapability()->setUiHidden( true );
    m_objectiveValuesSummaryAddresses.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_objectiveValuesSelectSummaryAddressPushButton,
                                "SelectObjectiveSummaryAddress",
                                "",
                                "",
                                "",
                                "" );
    caf::PdmUiPushButtonEditor::configureEditorForField( &m_objectiveValuesSelectSummaryAddressPushButton );
    m_objectiveValuesSelectSummaryAddressPushButton.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_objectiveValuesSelectSummaryAddressPushButton = false;

    CAF_PDM_InitFieldNoDefault( &m_objectiveFunction, "ObjectiveFunction", "Objective Function", "", "", "" );
    m_objectiveFunction.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_customObjectiveFunction, "CustomObjectiveFunction", "Custom Objective Function", "", "", "" );
    m_customObjectiveFunction.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_minValue, "MinValue", m_lowerLimit, "Min", "", "", "" );
    m_minValue.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_maxValue, "MaxValue", m_upperLimit, "Max", "", "", "" );
    m_maxValue.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_categories, "Categories", "Categories", "", "", "" );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveFilter::RimEnsembleCurveFilter( const QString& ensembleParameterName )
    : RimEnsembleCurveFilter()
{
    m_ensembleParameterName = ensembleParameterName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleCurveFilter::isActive() const
{
    RimEnsembleCurveFilterCollection* coll;
    firstAncestorOrThisOfType( coll );

    return ( !coll || coll->isActive() ) && m_active;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimEnsembleCurveFilter::minValue() const
{
    return m_minValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimEnsembleCurveFilter::maxValue() const
{
    return m_maxValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString> RimEnsembleCurveFilter::categories() const
{
    const auto cs = m_categories();
    return std::set<QString>( cs.begin(), cs.end() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEnsembleCurveFilter::ensembleParameterName() const
{
    return m_ensembleParameterName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEnsembleCurveFilter::filterId() const
{
    return QString( "%1" ).arg( (int64_t)this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEnsembleCurveFilter::description() const
{
    QString descriptor;
    if ( m_filterMode() == FilterMode::BY_ENSEMBLE_PARAMETER )
    {
        descriptor = QString( "%0" ).arg( m_ensembleParameterName() );
    }
    else if ( m_filterMode() == FilterMode::BY_OBJECTIVE_FUNCTION )
    {
        std::vector<RifEclipseSummaryAddress> addressVector;
        for ( RimSummaryAddress* address : m_objectiveValuesSummaryAddresses )
        {
            addressVector.push_back( address->address() );
        }
        descriptor =
            QString( "%1::%2%3%4" )
                .arg( caf::AppEnum<RimObjectiveFunction::FunctionType>( m_objectiveFunction() ).uiText() )
                .arg( addressVector.size() > 1 ? "(" : "" )
                .arg( QString::fromStdString( RifEclipseSummaryAddress::generateStringFromAddresses( addressVector, "+" ) ) )
                .arg( addressVector.size() > 1 ? ")" : "" );
    }
    else if ( m_filterMode() == FilterMode::BY_CUSTOM_OBJECTIVE_FUNCTION )
    {
        if ( m_customObjectiveFunction() && m_customObjectiveFunction()->isValid() )
        {
            descriptor = m_customObjectiveFunction()->title();
        }
        else
        {
            descriptor = "(Invalid Objective Function)";
        }
    }
    return QString( "%0 : %1 - %2" ).arg( descriptor ).arg( QString::number( m_minValue() ) ).arg( QString::number( m_maxValue() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RifEclipseSummaryAddress> RimEnsembleCurveFilter::summaryAddresses() const
{
    std::vector<RifEclipseSummaryAddress> addresses;
    for ( auto address : m_objectiveValuesSummaryAddresses() )
    {
        addresses.push_back( address->address() );
    }
    return addresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilter::setSummaryAddresses( std::vector<RifEclipseSummaryAddress> addresses )
{
    m_objectiveValuesSummaryAddresses.clear();
    for ( auto address : addresses )
    {
        RimSummaryAddress* summaryAddress = new RimSummaryAddress();
        summaryAddress->setAddress( address );
        m_objectiveValuesSummaryAddresses.push_back( summaryAddress );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimEnsembleCurveFilter::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_ensembleParameterName )
    {
        auto curveSet = parentCurveSet();
        if ( curveSet )
        {
            auto params = curveSet->correlationSortedEnsembleParameters();
            for ( const auto& [param, corr] : params )
            {
                options.push_back(
                    caf::PdmOptionItemInfo( QString( "%1 (Avg. correlation: %2)" ).arg( param.name ).arg( corr ),
                                            param.name ) );
            }
        }
    }
    else if ( fieldNeedingOptions == &m_categories )
    {
        auto curveSet = parentCurveSet();
        auto ensemble = curveSet ? curveSet->summaryCaseCollection() : nullptr;
        auto eParam   = ensemble ? ensemble->ensembleParameter( m_ensembleParameterName ) : EnsembleParameter();

        if ( eParam.isText() )
        {
            for ( const auto& val : eParam.values )
            {
                options.push_back( caf::PdmOptionItemInfo( val.toString(), val.toString() ) );
            }
        }
    }
    else if ( fieldNeedingOptions == &m_customObjectiveFunction )
    {
        for ( auto objFunc : parentCurveSet()->customObjectiveFunctionCollection()->objectiveFunctions() )
        {
            options.push_back( caf::PdmOptionItemInfo( objFunc->title(), objFunc ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilter::updateAddressesUiField()
{
    std::vector<RifEclipseSummaryAddress> addressVector;
    for ( RimSummaryAddress* address : m_objectiveValuesSummaryAddresses )
    {
        addressVector.push_back( address->address() );
    }
    m_objectiveValuesSummaryAddressesUiField =
        QString::fromStdString( RifEclipseSummaryAddress::generateStringFromAddresses( addressVector ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilter::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                               const QVariant&            oldValue,
                                               const QVariant&            newValue )
{
    auto curveSet = parentCurveSet();

    if ( changedField == &m_ensembleParameterName )
    {
        auto eParam = selectedEnsembleParameter();
        if ( eParam.isNumeric() )
        {
            updateMaxMinAndDefaultValues( true );
        }
        else if ( eParam.isText() )
        {
            m_categories.v().clear();
            for ( const auto& val : eParam.values )
            {
                m_categories.v().push_back( val.toString() );
            }
        }
        curveSet->updateAllCurves();

        auto collection = parentCurveFilterCollection();
        if ( collection ) collection->updateConnectedEditors();
    }
    else if ( changedField == &m_objectiveFunction )
    {
        curveSet->updateAllCurves();

        auto collection = parentCurveFilterCollection();
        if ( collection ) collection->updateConnectedEditors();
        updateMaxMinAndDefaultValues( true );
    }
    else if ( changedField == &m_customObjectiveFunction )
    {
        curveSet->updateAllCurves();

        auto collection = parentCurveFilterCollection();
        if ( collection ) collection->updateConnectedEditors();
        updateMaxMinAndDefaultValues( true );
    }
    else if ( changedField == &m_filterMode )
    {
        if ( m_objectiveValuesSummaryAddresses.size() == 0 )
        {
            RimSummaryAddress* summaryAddress = new RimSummaryAddress();
            summaryAddress->setAddress( parentCurveSet()->summaryAddress() );
            m_objectiveValuesSummaryAddresses.push_back( summaryAddress );
            updateAddressesUiField();
        }
        updateMaxMinAndDefaultValues( true );
    }
    else if ( changedField == &m_active || changedField == &m_minValue || changedField == &m_maxValue ||
              changedField == &m_categories )
    {
        if ( curveSet )
        {
            curveSet->updateAllCurves();
            curveSet->filterCollection()->updateConnectedEditors();
        }
    }
    else if ( changedField == &m_objectiveValuesSelectSummaryAddressPushButton )
    {
        RiuSummaryVectorSelectionDialog dlg( nullptr );
        dlg.enableMultiSelect( true );
        RimSummaryCaseCollection* candidateEnsemble = parentCurveSet()->summaryCaseCollection();

        std::vector<RifEclipseSummaryAddress> candidateAddresses;
        for ( auto address : m_objectiveValuesSummaryAddresses().childObjects() )
        {
            candidateAddresses.push_back( address->address() );
        }

        dlg.hideSummaryCases();
        dlg.setEnsembleAndAddresses( candidateEnsemble, candidateAddresses );

        if ( dlg.exec() == QDialog::Accepted )
        {
            auto curveSelection = dlg.curveSelection();
            if ( !curveSelection.empty() )
            {
                for ( auto address : curveSelection )
                {
                    RimSummaryAddress* summaryAddress = new RimSummaryAddress();
                    summaryAddress->setAddress( parentCurveSet()->summaryAddress() );
                    m_objectiveValuesSummaryAddresses.push_back( summaryAddress );
                }
                this->loadDataAndUpdate();
            }
        }

        m_objectiveValuesSelectSummaryAddressPushButton = false;
    }

    parentCurveSet()->updateFilterLegend();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimEnsembleCurveFilter::userDescriptionField()
{
    updateIcon();
    return &m_filterTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilter::updateIcon()
{
    QString resourceString;
    if ( m_filterMode() == FilterMode::BY_ENSEMBLE_PARAMETER )
    {
        resourceString = ":/FilterParameter.svg";
    }
    else if ( m_filterMode() == FilterMode::BY_OBJECTIVE_FUNCTION )
    {
        resourceString = ":/FilterFunction.svg";
    }
    setUiIconFromResourceString( resourceString );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilter::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto eParam = selectedEnsembleParameter();

    uiOrdering.add( &m_active );

    uiOrdering.add( &m_filterMode );

    if ( m_filterMode() == FilterMode::BY_ENSEMBLE_PARAMETER )
    {
        uiOrdering.add( &m_ensembleParameterName );
    }
    else if ( m_filterMode() == FilterMode::BY_OBJECTIVE_FUNCTION )
    {
        uiOrdering.add( &m_objectiveValuesSummaryAddressesUiField );
        uiOrdering.add( &m_objectiveValuesSelectSummaryAddressPushButton, { false, 1, 0 } );
        uiOrdering.add( &m_objectiveFunction );
    }
    else if ( m_filterMode() == FilterMode::BY_CUSTOM_OBJECTIVE_FUNCTION )
    {
        uiOrdering.add( &m_customObjectiveFunction );
    }

    if ( eParam.isNumeric() )
    {
        uiOrdering.add( &m_minValue );
        uiOrdering.add( &m_maxValue );
    }
    else if ( eParam.isText() )
    {
        uiOrdering.add( &m_categories );
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilter::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                    QString                    uiConfigName,
                                                    caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_minValue || field == &m_maxValue )
    {
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute );
        if ( !myAttr )
        {
            return;
        }

        myAttr->m_minimum = m_lowerLimit;
        myAttr->m_maximum = m_upperLimit;
    }
    else if ( field == &m_objectiveValuesSelectSummaryAddressPushButton )
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( attrib )
        {
            attrib->m_buttonText = "...";
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RimEnsembleCurveFilter::applyFilter( const std::vector<RimSummaryCase*>& allSumCases )
{
    auto curveSet = parentCurveSet();
    auto ensemble = curveSet ? curveSet->summaryCaseCollection() : nullptr;
    if ( !ensemble || !isActive() ) return allSumCases;

    std::set<RimSummaryCase*> casesToRemove;
    for ( const auto& sumCase : allSumCases )
    {
        if ( m_filterMode() == FilterMode::BY_ENSEMBLE_PARAMETER )
        {
            auto eParam = ensemble->ensembleParameter( m_ensembleParameterName() );
            if ( !eParam.isValid() ) continue;
            if ( !sumCase->caseRealizationParameters() ) continue;

            auto crpValue = sumCase->caseRealizationParameters()->parameterValue( m_ensembleParameterName() );

            if ( eParam.isNumeric() )
            {
                if ( !crpValue.isNumeric() || crpValue.numericValue() < m_minValue() ||
                     crpValue.numericValue() > m_maxValue() )
                {
                    casesToRemove.insert( sumCase );
                }
            }
            else if ( eParam.isText() )
            {
                const auto& filterCategories = categories();
                if ( !crpValue.isText() ||
                     std::count( filterCategories.begin(), filterCategories.end(), crpValue.textValue() ) == 0 )
                {
                    casesToRemove.insert( sumCase );
                }
            }
        }
        else if ( m_filterMode() == FilterMode::BY_OBJECTIVE_FUNCTION )
        {
            auto objectiveFunction = ensemble->objectiveFunction( m_objectiveFunction() );
            bool hasWarning        = false;

            std::vector<RifEclipseSummaryAddress> addresses;
            for ( auto address : m_objectiveValuesSummaryAddresses() )
            {
                addresses.push_back( address->address() );
            }

            double value = objectiveFunction->value( sumCase, addresses, &hasWarning );
            if ( hasWarning ) continue;

            if ( value < m_minValue() || value > m_maxValue )
            {
                casesToRemove.insert( sumCase );
            }
        }
        else if ( m_filterMode() == FilterMode::BY_CUSTOM_OBJECTIVE_FUNCTION )
        {
            if ( m_customObjectiveFunction() && m_customObjectiveFunction()->isValid() )
            {
                double value = m_customObjectiveFunction()->value( sumCase );
                if ( value < m_minValue() || value > m_maxValue )
                {
                    casesToRemove.insert( sumCase );
                }
            }
        }
    }

    std::vector<RimSummaryCase*> filteredCases;
    std::set<RimSummaryCase*>    allCasesSet( allSumCases.begin(), allSumCases.end() );
    std::set_difference( allCasesSet.begin(),
                         allCasesSet.end(),
                         casesToRemove.begin(),
                         casesToRemove.end(),
                         std::inserter( filteredCases, filteredCases.end() ) );
    return filteredCases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilter::loadDataAndUpdate()
{
    updateAddressesUiField();
    updateMaxMinAndDefaultValues( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimEnsembleCurveFilter::objectToggleField()
{
    return &m_active;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveSet* RimEnsembleCurveFilter::parentCurveSet() const
{
    RimEnsembleCurveSet* curveSet;
    firstAncestorOrThisOfType( curveSet );
    return curveSet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveFilterCollection* RimEnsembleCurveFilter::parentCurveFilterCollection() const
{
    RimEnsembleCurveFilterCollection* coll;
    firstAncestorOrThisOfType( coll );
    return coll;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilter::updateMaxMinAndDefaultValues( bool forceDefault )
{
    if ( m_filterMode() == FilterMode::BY_ENSEMBLE_PARAMETER )
    {
        if ( !selectedEnsembleParameter().isValid() )
        {
            auto ensParams = parentCurveSet()->correlationSortedEnsembleParameters();
            if ( !ensParams.empty() )
            {
                m_ensembleParameterName = ensParams.front().first.name;
            }
        }

        auto eParam = selectedEnsembleParameter();
        if ( eParam.isValid() && eParam.isNumeric() )
        {
            if ( RiaCurveDataTools::isValidValue( eParam.minValue, false ) ) m_lowerLimit = eParam.minValue;
            if ( RiaCurveDataTools::isValidValue( eParam.maxValue, false ) ) m_upperLimit = eParam.maxValue;

            if ( forceDefault || !( m_minValue >= m_lowerLimit && m_minValue <= m_upperLimit ) )
                m_minValue = m_lowerLimit;
            if ( forceDefault || !( m_maxValue >= m_lowerLimit && m_maxValue <= m_upperLimit ) )
                m_maxValue = m_upperLimit;

            m_minValue.uiCapability()->setUiName( QString( "Min (%1)" ).arg( m_lowerLimit ) );
            m_maxValue.uiCapability()->setUiName( QString( "Max (%1)" ).arg( m_upperLimit ) );
        }
    }
    else if ( m_filterMode() == FilterMode::BY_OBJECTIVE_FUNCTION )
    {
        auto objectiveFunction = parentCurveSet()->summaryCaseCollection()->objectiveFunction( m_objectiveFunction() );
        std::vector<RifEclipseSummaryAddress> addresses;
        for ( auto address : m_objectiveValuesSummaryAddresses() )
        {
            addresses.push_back( address->address() );
        }
        if ( objectiveFunction->isValid( addresses ) )
        {
            std::pair<double, double> minMaxValues = objectiveFunction->minMaxValues( addresses );

            m_lowerLimit = minMaxValues.first;
            m_upperLimit = minMaxValues.second;

            if ( forceDefault || !( m_minValue >= m_lowerLimit && m_minValue <= m_upperLimit ) )
                m_minValue = m_lowerLimit;
            if ( forceDefault || !( m_maxValue >= m_lowerLimit && m_maxValue <= m_upperLimit ) )
                m_maxValue = m_upperLimit;

            m_minValue.uiCapability()->setUiName( QString( "Min (%1)" ).arg( m_lowerLimit ) );
            m_maxValue.uiCapability()->setUiName( QString( "Max (%1)" ).arg( m_upperLimit ) );
        }
    }
    else if ( m_filterMode() == FilterMode::BY_CUSTOM_OBJECTIVE_FUNCTION )
    {
        if ( m_customObjectiveFunction() && m_customObjectiveFunction()->isValid() )
        {
            std::pair<double, double> minMaxValues = m_customObjectiveFunction->minMaxValues();

            m_lowerLimit = minMaxValues.first;
            m_upperLimit = minMaxValues.second;

            if ( forceDefault || !( m_minValue >= m_lowerLimit && m_minValue <= m_upperLimit ) )
                m_minValue = m_lowerLimit;
            if ( forceDefault || !( m_maxValue >= m_lowerLimit && m_maxValue <= m_upperLimit ) )
                m_maxValue = m_upperLimit;

            m_minValue.uiCapability()->setUiName( QString( "Min (%1)" ).arg( m_lowerLimit ) );
            m_maxValue.uiCapability()->setUiName( QString( "Max (%1)" ).arg( m_upperLimit ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
EnsembleParameter RimEnsembleCurveFilter::selectedEnsembleParameter() const
{
    auto curveSet = parentCurveSet();
    auto ensemble = curveSet ? curveSet->summaryCaseCollection() : nullptr;
    return ensemble ? ensemble->ensembleParameter( m_ensembleParameterName ) : EnsembleParameter();
}
