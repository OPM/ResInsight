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
#include "RiaNumericalTools.h"
#include "RiaStdStringTools.h"
#include "Summary/RiaSummaryCurveDefinition.h"

#include "RimCustomObjectiveFunction.h"
#include "RimCustomObjectiveFunctionCollection.h"
#include "RimEnsembleCurveFilterCollection.h"
#include "RimEnsembleCurveSet.h"
#include "RimObjectiveFunctionTools.h"
#include "RimProject.h"
#include "RimSummaryAddress.h"
#include "RimSummaryAddressSelector.h"
#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"

#include "RiuSummaryVectorSelectionDialog.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"
#include "cafPdmUiValueRangeEditor.h"

namespace caf
{
template <>
void caf::AppEnum<RimEnsembleCurveFilter::FilterMode>::setUp()
{
    addItem( RimEnsembleCurveFilter::FilterMode::ENSEMBLE_PARAMETER, "ENSEMBLE_PARAMETER", "By Ensemble Parameter", { "BY_ENSEMBLE_PARAMETER" } );
    addItem( RimEnsembleCurveFilter::FilterMode::SUMMARY_VALUE, "SUMMARY_VALUE", "By Summary Value" );
    addItem( RimEnsembleCurveFilter::FilterMode::OBJECTIVE_FUNCTION, "OBJECTIVE_FUNCTION", "By Objective Function", { "BY_OBJECTIVE_FUNCTION" } );
    addItem( RimEnsembleCurveFilter::FilterMode::CUSTOM_OBJECTIVE_FUNCTION,
             "CUSTOM_OBJECTIVE_FUNCTION",
             "By Custom Objective Function",
             { "By_CUSTOM_OBJECTIVE_FUNCTION" } );

    setDefault( RimEnsembleCurveFilter::FilterMode::SUMMARY_VALUE );
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
    CAF_PDM_InitObject( "Ensemble Curve Filter", ":/Filter.svg" );

    CAF_PDM_InitFieldNoDefault( &m_filterTitle, "FilterTitle", "Title" );
    m_filterTitle.registerGetMethod( this, &RimEnsembleCurveFilter::description );
    m_filterTitle.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_active, "Active", "Active" );
    m_active = true;

    CAF_PDM_InitFieldNoDefault( &m_filterMode, "FilterMode", "Filter Mode" );

    CAF_PDM_InitFieldNoDefault( &m_ensembleParameterName, "EnsembleParameter", "Ensemble Parameter" );
    m_ensembleParameterName.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_objectiveValuesSummaryAddressesUiField, "SelectedObjectiveSummaryVar", "Vector" );
    m_objectiveValuesSummaryAddressesUiField.xmlCapability()->disableIO();
    m_objectiveValuesSummaryAddressesUiField.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_objectiveValuesSummaryAddresses, "ObjectiveSummaryAddress", "Summary Address" );
    m_objectiveValuesSummaryAddresses.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_objectiveValuesSelectSummaryAddressPushButton, "SelectObjectiveSummaryAddress", "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_objectiveValuesSelectSummaryAddressPushButton );
    m_objectiveValuesSelectSummaryAddressPushButton = false;

    CAF_PDM_InitFieldNoDefault( &m_objectiveFunction, "ObjectiveFunction", "Objective Function" );
    m_objectiveFunction = new RimObjectiveFunction();
    m_objectiveFunction.uiCapability()->setUiTreeChildrenHidden( true );
    m_objectiveFunction->changed.connect( this, &RimEnsembleCurveFilter::onObjectionFunctionChanged );

    CAF_PDM_InitFieldNoDefault( &m_customObjectiveFunction, "CustomObjectiveFunction", "Custom Objective Function" );
    m_customObjectiveFunction.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_minValue_OBSOLETE, "MinValue", m_lowerLimit, "Min" );
    m_minValue_OBSOLETE.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );
    m_minValue_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitField( &m_maxValue_OBSOLETE, "MaxValue", m_upperLimit, "Max" );
    m_maxValue_OBSOLETE.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );
    m_maxValue_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitField( &m_valueRange, "ValueRange", std::pair( 0.0, 0.0 ), "Value Range" );
    m_valueRange.uiCapability()->setUiEditorTypeName( caf::PdmUiValueRangeEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_categories, "Categories", "Categories" );

    CAF_PDM_InitFieldNoDefault( &m_realizationFilter, "RealizationFilter", "Realization Filter" );

    CAF_PDM_InitFieldNoDefault( &m_addressSelector, "AddressSelector", "" );
    m_addressSelector = new RimSummaryAddressSelector;
    m_addressSelector->setShowDataSource( false );
    m_addressSelector->setShowResampling( false );
    m_addressSelector->setShowAxis( false );
    m_addressSelector.uiCapability()->setUiTreeChildrenHidden( true );

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
    auto coll = firstAncestorOrThisOfType<RimEnsembleCurveFilterCollection>();

    return ( !coll || coll->isActive() ) && m_active;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilter::setActive( bool active )
{
    m_active = active;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimEnsembleCurveFilter::minValue() const
{
    return m_valueRange().first;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimEnsembleCurveFilter::maxValue() const
{
    return m_valueRange().second;
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
    if ( m_filterMode() == FilterMode::ENSEMBLE_PARAMETER )
    {
        if ( m_ensembleParameterName() == RiaDefines::summaryRealizationNumber() )
        {
            return "Realizations : " + m_realizationFilter;
        }

        descriptor = QString( "%0" ).arg( m_ensembleParameterName() );
    }
    else if ( m_filterMode() == FilterMode::OBJECTIVE_FUNCTION )
    {
        std::vector<RifEclipseSummaryAddress> addressVector;
        for ( RimSummaryAddress* address : m_objectiveValuesSummaryAddresses )
        {
            addressVector.push_back( address->address() );
        }
        descriptor = QString( "%1::%2%3%4" )
                         .arg( m_objectiveFunction()->shortName() )
                         .arg( addressVector.size() > 1 ? "(" : "" )
                         .arg( QString::fromStdString( RifEclipseSummaryAddress::generateStringFromAddresses( addressVector, "+" ) ) )
                         .arg( addressVector.size() > 1 ? ")" : "" );
    }
    else if ( m_filterMode() == FilterMode::CUSTOM_OBJECTIVE_FUNCTION )
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
    else if ( m_filterMode() == FilterMode::SUMMARY_VALUE )
    {
        auto adr   = m_addressSelector->summaryAddress();
        descriptor = QString::fromStdString( adr.toEclipseTextAddress() );
    }

    char format    = 'g';
    int  precision = 2;

    return QString( "%0 : %1 - %2" )
        .arg( descriptor )
        .arg( QString::number( minValue(), format, precision ) )
        .arg( QString::number( maxValue(), format, precision ) );
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
    m_objectiveValuesSummaryAddresses.deleteChildren();
    for ( auto address : addresses )
    {
        RimSummaryAddress* summaryAddress = new RimSummaryAddress();
        summaryAddress->setAddress( address );
        m_objectiveValuesSummaryAddresses.push_back( summaryAddress );

        m_addressSelector->setAddress( address );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveFilter::FilterMode RimEnsembleCurveFilter::filterMode() const
{
    return m_filterMode();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimEnsembleCurveFilter::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_ensembleParameterName )
    {
        auto curveSet = parentCurveSet();
        if ( curveSet )
        {
            auto params = curveSet->ensembleParameters( RimCurveAppearanceDefines::ParameterSorting::ABSOLUTE_VALUE );
            for ( const auto& [param, corr] : params )
            {
                options.push_back( caf::PdmOptionItemInfo( QString( "%1 (Avg. correlation: %2)" ).arg( param.name ).arg( corr ), param.name ) );
            }
        }
    }
    else if ( fieldNeedingOptions == &m_categories )
    {
        auto curveSet = parentCurveSet();
        auto ensemble = curveSet ? curveSet->summaryEnsemble() : nullptr;
        auto eParam   = ensemble ? ensemble->ensembleParameter( m_ensembleParameterName ) : RigEnsembleParameter();

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
    m_objectiveValuesSummaryAddressesUiField = QString::fromStdString( RifEclipseSummaryAddress::generateStringFromAddresses( addressVector ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilter::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
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
        if ( m_objectiveValuesSummaryAddresses.empty() )
        {
            RimSummaryAddress* summaryAddress = new RimSummaryAddress();

            RifEclipseSummaryAddress candidateAdr = parentCurveSet()->summaryAddressY();

            auto nativeQuantityName = RimObjectiveFunctionTools::nativeQuantityName( candidateAdr.vectorName() );
            candidateAdr.setVectorName( nativeQuantityName );
            summaryAddress->setAddress( candidateAdr );
            m_objectiveValuesSummaryAddresses.push_back( summaryAddress );
            updateAddressesUiField();
        }
        updateMaxMinAndDefaultValues( true );
    }
    else if ( changedField == &m_active || changedField == &m_valueRange || changedField == &m_categories ||
              changedField == &m_realizationFilter )
    {
        if ( curveSet )
        {
            curveSet->updateAllCurves();
            curveSet->onFilterChanged();
            curveSet->filterCollection()->updateConnectedEditors();
        }
    }
    else if ( changedField == &m_objectiveValuesSelectSummaryAddressPushButton )
    {
        RiuSummaryVectorSelectionDialog dlg( nullptr );
        RimObjectiveFunctionTools::configureDialogForObjectiveFunctions( &dlg );
        RimSummaryEnsemble* candidateEnsemble = parentCurveSet()->summaryEnsemble();

        std::vector<RifEclipseSummaryAddress> candidateAddresses;
        for ( auto address : m_objectiveValuesSummaryAddresses().childrenByType() )
        {
            candidateAddresses.push_back( address->address() );
        }

        dlg.setEnsembleAndAddresses( candidateEnsemble, candidateAddresses );

        if ( dlg.exec() == QDialog::Accepted )
        {
            auto curveSelection = dlg.curveSelection();
            if ( !curveSelection.empty() )
            {
                m_objectiveValuesSummaryAddresses.deleteChildren();
                for ( auto address : curveSelection )
                {
                    RimSummaryAddress* summaryAddress = new RimSummaryAddress();
                    summaryAddress->setAddress( address.summaryAddressY() );
                    m_objectiveValuesSummaryAddresses.push_back( summaryAddress );
                }
                loadDataAndUpdate();
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
    if ( m_filterMode() == FilterMode::ENSEMBLE_PARAMETER )
    {
        resourceString = ":/FilterParameter.svg";
    }
    else if ( m_filterMode() == FilterMode::OBJECTIVE_FUNCTION )
    {
        resourceString = ":/FilterFunction.svg";
    }
    setUiIconFromResourceString( resourceString );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilter::updateCurves()
{
    if ( auto curveSet = parentCurveSet() )
    {
        curveSet->updateAllCurves();
        curveSet->updateFilterLegend();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilter::initAfterRead()
{
    if ( RimProject::current()->isProjectFileVersionEqualOrOlderThan( "2025.04.0" ) )
    {
        m_valueRange = std::pair<double, double>( m_minValue_OBSOLETE, m_maxValue_OBSOLETE );
    };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilter::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto eParam = selectedEnsembleParameter();

    uiOrdering.add( &m_active );

    uiOrdering.add( &m_filterMode );

    if ( m_filterMode() == FilterMode::ENSEMBLE_PARAMETER )
    {
        uiOrdering.add( &m_ensembleParameterName );
    }
    else if ( m_filterMode() == FilterMode::OBJECTIVE_FUNCTION )
    {
        uiOrdering.add( &m_objectiveValuesSummaryAddressesUiField );
        uiOrdering.add( &m_objectiveValuesSelectSummaryAddressPushButton, { .newRow = false, .totalColumnSpan = 1, .leftLabelColumnSpan = 0 } );
        {
            auto equationGroup = uiOrdering.addNewGroup( "Equation" );
            m_objectiveFunction->uiOrdering( "", *equationGroup );
        }

        uiOrdering.add( &m_objectiveFunction );
    }
    else if ( m_filterMode() == FilterMode::CUSTOM_OBJECTIVE_FUNCTION )
    {
        uiOrdering.add( &m_customObjectiveFunction );
    }
    else if ( m_filterMode() == FilterMode::SUMMARY_VALUE )
    {
        if ( auto curveSet = parentCurveSet() )
        {
            m_addressSelector->setEnsemble( curveSet->summaryEnsemble() );
        }
        m_addressSelector->uiOrdering( uiConfigName, uiOrdering );
    }

    if ( m_ensembleParameterName() == RiaDefines::summaryRealizationNumber() )
    {
        uiOrdering.add( &m_realizationFilter );
    }
    else if ( eParam.isNumeric() || m_filterMode() == FilterMode::SUMMARY_VALUE )
    {
        uiOrdering.add( &m_valueRange );
    }
    else if ( eParam.isText() )
    {
        uiOrdering.add( &m_categories );
    }

    if ( auto curveSet = parentCurveSet() )
    {
        curveSet->appendTimeGroup( uiOrdering );
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilter::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_objectiveValuesSelectSummaryAddressPushButton )
    {
        if ( auto attr = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute ) )
        {
            attr->m_buttonText = "...";
        }
    }
    else if ( field == &m_valueRange )
    {
        if ( auto attr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute ) )
        {
            attr->m_sliderTickCount = 100;

            attr->m_minimum = m_lowerLimit;
            attr->m_maximum = m_upperLimit;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RimEnsembleCurveFilter::applyFilter( const std::vector<RimSummaryCase*>& allSumCases )
{
    auto curveSet = parentCurveSet();
    auto ensemble = curveSet ? curveSet->summaryEnsemble() : nullptr;
    if ( !ensemble || !isActive() ) return allSumCases;

    bool          useIntegerSelection = false;
    std::set<int> integerSelection;

    if ( m_ensembleParameterName() == RiaDefines::summaryRealizationNumber() )
    {
        auto eParam   = selectedEnsembleParameter();
        int  minValue = eParam.minValue;
        int  maxValue = eParam.maxValue;

        integerSelection    = RiaStdStringTools::valuesFromRangeSelection( m_realizationFilter().toStdString(), minValue, maxValue );
        useIntegerSelection = true;
    }

    std::set<RimSummaryCase*> casesToRemove;
    for ( const auto& sumCase : allSumCases )
    {
        if ( m_filterMode() == FilterMode::ENSEMBLE_PARAMETER )
        {
            auto eParam = ensemble->ensembleParameter( m_ensembleParameterName() );
            if ( !eParam.isValid() ) continue;
            if ( !sumCase->caseRealizationParameters() ) continue;

            auto crpValue = sumCase->caseRealizationParameters()->parameterValue( m_ensembleParameterName() );

            if ( useIntegerSelection )
            {
                int integerValue = crpValue.numericValue();

                if ( !integerSelection.contains( integerValue ) )
                {
                    casesToRemove.insert( sumCase );
                }
            }
            else if ( eParam.isNumeric() )
            {
                if ( !crpValue.isNumeric() || !RiaNumericalTools::isValueInRange( crpValue.numericValue(), m_valueRange() ) )
                {
                    casesToRemove.insert( sumCase );
                }
            }
            else if ( eParam.isText() )
            {
                const auto& filterCategories = categories();
                if ( !crpValue.isText() || std::count( filterCategories.begin(), filterCategories.end(), crpValue.textValue() ) == 0 )
                {
                    casesToRemove.insert( sumCase );
                }
            }
        }
        else if ( m_filterMode() == FilterMode::OBJECTIVE_FUNCTION )
        {
            bool hasWarning = false;

            std::vector<RifEclipseSummaryAddress> addresses;
            for ( auto address : m_objectiveValuesSummaryAddresses() )
            {
                addresses.push_back( address->address() );
            }

            double value = m_objectiveFunction->value( sumCase, addresses, curveSet->objectiveFunctionTimeConfig(), &hasWarning );
            if ( hasWarning ) continue;

            if ( !RiaNumericalTools::isValueInRange( value, m_valueRange() ) )
            {
                casesToRemove.insert( sumCase );
            }
        }
        else if ( m_filterMode() == FilterMode::CUSTOM_OBJECTIVE_FUNCTION )
        {
            if ( m_customObjectiveFunction() && m_customObjectiveFunction()->isValid() )
            {
                double value = m_customObjectiveFunction()->value( sumCase );
                if ( !RiaNumericalTools::isValueInRange( value, m_valueRange() ) )
                {
                    casesToRemove.insert( sumCase );
                }
            }
        }
        else if ( m_filterMode() == FilterMode::SUMMARY_VALUE )
        {
            if ( auto reader = sumCase->summaryReader() )
            {
                const auto [isValid, values] = reader->values( m_addressSelector->summaryAddress() );

                bool isInsideFilter = isValid;

                if ( isValid && !values.empty() )
                {
                    auto timeConfig = curveSet->objectiveFunctionTimeConfig();
                    auto timeSteps  = reader->timeSteps( m_addressSelector->summaryAddress() );

                    for ( size_t i = 0; i < std::min( timeSteps.size(), values.size() ); i++ )
                    {
                        if ( timeSteps[i] < timeConfig.m_startTimeStep || timeSteps[i] > timeConfig.m_endTimeStep ) continue;

                        isInsideFilter = RiaNumericalTools::isValueInRange( values[i], m_valueRange() );
                    }
                }
                if ( !isInsideFilter )
                {
                    casesToRemove.insert( sumCase );
                }
            }
        }
    }

    std::vector<RimSummaryCase*> filteredCases;
    for ( auto summaryCase : allSumCases )
    {
        if ( !casesToRemove.contains( summaryCase ) )
        {
            filteredCases.push_back( summaryCase );
        }
    }
    return filteredCases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilter::loadDataAndUpdate()
{
    updateAddressesUiField();
    updateMaxMinAndDefaultValues( false );
    updateCurves();
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
void RimEnsembleCurveFilter::childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField )
{
    updateMaxMinAndDefaultValues( true );
    updateCurves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilter::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    if ( isActive() )
    {
        menuBuilder << "RicCreateEnsembleFromFilteredCasesFeature";
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveSet* RimEnsembleCurveFilter::parentCurveSet() const
{
    return firstAncestorOrThisOfType<RimEnsembleCurveSet>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilter::onObjectionFunctionChanged( const caf::SignalEmitter* emitter )
{
    updateMaxMinAndDefaultValues( false );
    updateCurves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilter::updateMaxMinAndDefaultValuesFromParent()
{
    updateMaxMinAndDefaultValues( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveFilterCollection* RimEnsembleCurveFilter::parentCurveFilterCollection() const
{
    return firstAncestorOrThisOfType<RimEnsembleCurveFilterCollection>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilter::updateMaxMinAndDefaultValues( bool forceDefault )
{
    if ( m_filterMode() == FilterMode::ENSEMBLE_PARAMETER )
    {
        if ( !selectedEnsembleParameter().isValid() )
        {
            auto ensParams = parentCurveSet()->ensembleParameters( RimCurveAppearanceDefines::ParameterSorting::ABSOLUTE_VALUE );
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

            if ( m_ensembleParameterName() == RiaDefines::summaryRealizationNumber() )
            {
                int lower = eParam.minValue;
                int upper = eParam.maxValue;

                m_realizationFilter.uiCapability()->setUiName( QString( "Integer Selection\n[%1..%2]" ).arg( lower ).arg( upper ) );

                if ( m_realizationFilter().isEmpty() )
                {
                    m_realizationFilter = QString( "%1-%2" ).arg( lower ).arg( upper );
                }
            }
        }
    }
    else if ( m_filterMode() == FilterMode::OBJECTIVE_FUNCTION )
    {
        auto                                  objectiveFunction = m_objectiveFunction();
        std::vector<RifEclipseSummaryAddress> addresses;
        for ( auto address : m_objectiveValuesSummaryAddresses() )
        {
            addresses.push_back( address->address() );
        }
        {
            auto summaryCases = parentCurveSet()->summaryEnsemble()->allSummaryCases();

            auto [minObjValue, maxObjValue] =
                objectiveFunction->minMaxValues( summaryCases, addresses, parentCurveSet()->objectiveFunctionTimeConfig() );

            m_lowerLimit = minObjValue;
            m_upperLimit = maxObjValue;
        }
    }
    else if ( m_filterMode() == FilterMode::CUSTOM_OBJECTIVE_FUNCTION )
    {
        if ( m_customObjectiveFunction() && m_customObjectiveFunction()->isValid() )
        {
            std::pair<double, double> minMaxValues = m_customObjectiveFunction->minMaxValues();

            m_lowerLimit = minMaxValues.first;
            m_upperLimit = minMaxValues.second;
        }
    }
    else if ( m_filterMode() == FilterMode::SUMMARY_VALUE )
    {
        if ( parentCurveSet() && parentCurveSet()->summaryEnsemble() )
        {
            auto ensemble = parentCurveSet()->summaryEnsemble();

            auto minMaxValues = ensemble->minMax( m_addressSelector->summaryAddress() );
            m_lowerLimit      = minMaxValues.first;
            m_upperLimit      = minMaxValues.second;
        }
    }

    if ( forceDefault )
    {
        m_valueRange = { m_lowerLimit, m_upperLimit };
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEnsembleParameter RimEnsembleCurveFilter::selectedEnsembleParameter() const
{
    auto curveSet = parentCurveSet();
    auto ensemble = curveSet ? curveSet->summaryEnsemble() : nullptr;
    return ensemble ? ensemble->ensembleParameter( m_ensembleParameterName ) : RigEnsembleParameter();
}
