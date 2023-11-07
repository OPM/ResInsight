/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RimGridCalculation.h"

#include "RiaDefines.h"
#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaPorosityModel.h"

#include "RigActiveCellInfo.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseTools.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseStatisticsCase.h"
#include "RimEclipseView.h"
#include "RimGridCalculationVariable.h"
#include "RimProject.h"
#include "RimReloadCaseTools.h"
#include "RimTools.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigGridManager.h"
#include "RigMainGrid.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"

#include "expressionparser/ExpressionParser.h"

#include "cafProgressInfo.h"

#include <QMessageBox>

CAF_PDM_SOURCE_INIT( RimGridCalculation, "RimGridCalculation" );

namespace caf
{
template <>
void caf::AppEnum<RimGridCalculation::DefaultValueType>::setUp()
{
    addItem( RimGridCalculation::DefaultValueType::POSITIVE_INFINITY, "POSITIVE_INFINITY", "Infinity" );
    addItem( RimGridCalculation::DefaultValueType::FROM_PROPERTY, "FROM_PROPERTY", "Property Value" );
    addItem( RimGridCalculation::DefaultValueType::USER_DEFINED, "USER_DEFINED", "User Defined Custom Value" );
    setDefault( RimGridCalculation::DefaultValueType::POSITIVE_INFINITY );
}
}; // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCalculation::RimGridCalculation()
{
    CAF_PDM_InitObject( "RimGridCalculation", ":/octave.png", "Calculation", "" );
    CAF_PDM_InitFieldNoDefault( &m_cellFilterView, "VisibleCellView", "Filter by 3d View Visibility" );
    CAF_PDM_InitFieldNoDefault( &m_defaultValueType, "DefaultValueType", "Non-visible Cell Value" );
    CAF_PDM_InitField( &m_defaultValue, "DefaultValue", 0.0, "Custom Value" );
    CAF_PDM_InitFieldNoDefault( &m_destinationCase, "DestinationCase", "Destination Case" );
    CAF_PDM_InitField( &m_allCases, "AllDestinationCase", false, "Apply to All Cases" );
    CAF_PDM_InitField( &m_defaultPropertyVariableIndex, "DefaultPropertyVariableName", 0, "Property Variable Name" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCalculation::preCalculate() const
{
    if ( RiaGuiApplication::isRunning() && m_allCases() )
    {
        auto reply = QMessageBox::question( nullptr,
                                            QString( "Grid Property Calculator" ),
                                            QString( "Calculation will be executed on all grid model cases. Do you want to continue? " ),
                                            QMessageBox::Yes | QMessageBox::No );

        if ( reply == QMessageBox::No ) return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCalculationVariable* RimGridCalculation::createVariable()
{
    auto variable = new RimGridCalculationVariable;

    variable->eclipseResultChanged.connect( this, &RimGridCalculation::onVariableUpdated );

    return variable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCalculation::calculate()
{
    auto timeSteps = std::nullopt;

    return calculateForCases( outputEclipseCases(), timeSteps );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseCase*> RimGridCalculation::outputEclipseCases() const
{
    if ( m_allCases )
    {
        // Find all Eclipse cases suitable for grid calculations. This includes all single grid cases and source cases in a grid case group.
        // Exclude the statistics cases, as it is not possible to use them in a grid calculations.
        //
        // Note that data read from file can be released from memory when statistics for a time step is calculated. See
        // RimEclipseStatisticsCaseEvaluator::evaluateForResults()

        return RimEclipseCaseTools::allEclipseGridCases();
    }

    return { m_destinationCase };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseCase*> RimGridCalculation::inputCases() const
{
    std::vector<RimEclipseCase*> cases;

    for ( const auto& variable : m_variables )
    {
        auto* v = dynamic_cast<RimGridCalculationVariable*>( variable.p() );

        if ( v->eclipseCase() ) cases.push_back( v->eclipseCase() );
    }

    return cases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCalculation::DefaultValueConfig RimGridCalculation::defaultValueConfiguration() const
{
    if ( m_defaultValueType() == RimGridCalculation::DefaultValueType::USER_DEFINED )
        return std::make_pair( m_defaultValueType(), m_defaultValue() );

    return std::make_pair( m_defaultValueType(), HUGE_VAL );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculation::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimUserDefinedCalculation::defineUiOrdering( uiConfigName, uiOrdering );

    uiOrdering.add( &m_destinationCase );

    uiOrdering.add( &m_allCases );
    if ( !allSourceCasesAreEqualToDestinationCase() )
    {
        m_allCases = false;
    }
    m_allCases.uiCapability()->setUiReadOnly( !allSourceCasesAreEqualToDestinationCase() );

    caf::PdmUiGroup* filterGroup = uiOrdering.addNewGroup( "Cell Filter" );
    filterGroup->setCollapsedByDefault();
    filterGroup->add( &m_cellFilterView );

    if ( m_cellFilterView() != nullptr )
    {
        filterGroup->add( &m_defaultValueType );

        if ( m_defaultValueType() == RimGridCalculation::DefaultValueType::FROM_PROPERTY )
            filterGroup->add( &m_defaultPropertyVariableIndex );
        else if ( m_defaultValueType() == RimGridCalculation::DefaultValueType::USER_DEFINED )
            filterGroup->add( &m_defaultValue );
    }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimGridCalculation::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_cellFilterView )
    {
        options.push_back( caf::PdmOptionItemInfo( "Disabled", nullptr ) );

        std::vector<Rim3dView*> views;
        RimProject::current()->allViews( views );

        RimEclipseCase* firstEclipseCase = nullptr;
        if ( !inputCases().empty() ) firstEclipseCase = inputCases().front();

        if ( firstEclipseCase )
        {
            for ( auto* view : views )
            {
                auto eclipseView = dynamic_cast<RimEclipseView*>( view );
                if ( !eclipseView ) continue;
                if ( !firstEclipseCase->isGridSizeEqualTo( eclipseView->eclipseCase() ) ) continue;

                options.push_back( caf::PdmOptionItemInfo( view->autoName(), view, false, view->uiIconProvider() ) );
            }
        }
    }
    else if ( fieldNeedingOptions == &m_destinationCase )
    {
        RimEclipseCase* firstInputCase = nullptr;

        if ( !inputCases().empty() )
        {
            firstInputCase = inputCases()[0];
        }

        for ( auto eclipseCase : RimEclipseCaseTools::allEclipseGridCases() )
        {
            if ( !eclipseCase ) continue;
            if ( firstInputCase && !firstInputCase->isGridSizeEqualTo( eclipseCase ) ) continue;

            options.push_back( caf::PdmOptionItemInfo( eclipseCase->caseUserDescription(), eclipseCase, false, eclipseCase->uiIconProvider() ) );
        }

        options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
    }
    else if ( fieldNeedingOptions == &m_defaultPropertyVariableIndex )
    {
        for ( int i = 0; i < static_cast<int>( m_variables.size() ); i++ )
        {
            auto v = dynamic_cast<RimGridCalculationVariable*>( m_variables[i] );

            QString optionText = v->name();
            options.push_back( caf::PdmOptionItemInfo( optionText, i ) );
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculation::initAfterRead()
{
    for ( auto& variable : m_variables )
    {
        auto gridVar = dynamic_cast<RimGridCalculationVariable*>( variable.p() );
        if ( gridVar )
        {
            gridVar->eclipseResultChanged.connect( this, &RimGridCalculation::onVariableUpdated );

            if ( m_destinationCase == nullptr ) m_destinationCase = gridVar->eclipseCase();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculation::onVariableUpdated( const SignalEmitter* emitter )
{
    if ( m_destinationCase == nullptr )
    {
        auto variable = dynamic_cast<const RimGridCalculationVariable*>( emitter );
        if ( variable && variable->eclipseCase() )
        {
            m_destinationCase = variable->eclipseCase();

            updateConnectedEditors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCalculation::allSourceCasesAreEqualToDestinationCase() const
{
    if ( m_destinationCase() == nullptr ) return false;

    for ( const auto& variable : m_variables )
    {
        auto gridVar = dynamic_cast<RimGridCalculationVariable*>( variable.p() );
        if ( gridVar )
        {
            if ( gridVar->eclipseCase() != m_destinationCase() ) return false;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseResultAddress RimGridCalculation::outputAddress() const
{
    QString                 leftHandSideVariableName = RimGridCalculation::findLeftHandSide( m_expression );
    RigEclipseResultAddress resAddr( RiaDefines::ResultCatType::GENERATED, leftHandSideVariableName );
    return resAddr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimGridCalculation::getInputVectorForVariable( RimGridCalculationVariable*   v,
                                                                   size_t                        tsId,
                                                                   RiaDefines::PorosityModelType porosityModel,
                                                                   RimEclipseCase*               outputEclipseCase ) const
{
    if ( !outputEclipseCase ) return {};

    const RigMainGrid* mainGrid = outputEclipseCase->mainGrid();
    if ( !mainGrid ) return {};

    int timeStep = v->timeStep();

    auto resultCategoryType = v->resultCategoryType();

    // General case is to use the data from the given time step
    size_t timeStepToUse = tsId;

    if ( resultCategoryType == RiaDefines::ResultCatType::STATIC_NATIVE )
    {
        // Use the first time step for static data for all time steps
        timeStepToUse = 0;
    }
    else if ( timeStep != RimGridCalculationVariable::allTimeStepsValue() )
    {
        // Use data from a specific time step for this variable for all result time steps
        timeStepToUse = timeStep;
    }

    RigEclipseResultAddress resAddr( resultCategoryType, v->resultVariable() );

    auto eclipseCaseData        = outputEclipseCase->eclipseCaseData();
    auto rigCaseCellResultsData = eclipseCaseData->results( porosityModel );
    if ( !rigCaseCellResultsData->ensureKnownResultLoaded( resAddr ) ) return {};

    size_t maxGridCount = mainGrid->gridCount();

    auto   activeCellInfo = eclipseCaseData->activeCellInfo( porosityModel );
    size_t cellCount      = activeCellInfo->reservoirActiveCellCount();

    std::vector<double> inputValues( cellCount );
    for ( size_t gridIdx = 0; gridIdx < maxGridCount; ++gridIdx )
    {
        auto grid = mainGrid->gridByIndex( gridIdx );

        cvf::ref<RigResultAccessor> sourceResultAccessor =
            RigResultAccessorFactory::createFromResultAddress( eclipseCaseData, gridIdx, porosityModel, timeStepToUse, resAddr );

#pragma omp parallel for
        for ( int localGridCellIdx = 0; localGridCellIdx < static_cast<int>( grid->cellCount() ); localGridCellIdx++ )
        {
            const size_t reservoirCellIndex = grid->reservoirCellIndex( localGridCellIdx );
            if ( activeCellInfo->isActive( reservoirCellIndex ) )
            {
                size_t cellResultIndex       = activeCellInfo->cellResultIndex( reservoirCellIndex );
                inputValues[cellResultIndex] = sourceResultAccessor->cellScalar( localGridCellIdx );
            }
        }
    }

    return inputValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculation::replaceFilteredValuesWithVector( const std::vector<double>&    inputValues,
                                                          cvf::ref<cvf::UByteArray>     visibility,
                                                          std::vector<double>&          resultValues,
                                                          RiaDefines::PorosityModelType porosityModel,
                                                          RimEclipseCase*               outputEclipseCase )

{
    auto activeCellInfo = outputEclipseCase->eclipseCaseData()->activeCellInfo( porosityModel );
    int  numCells       = static_cast<int>( visibility->size() );

#pragma omp parallel for
    for ( int i = 0; i < numCells; i++ )
    {
        if ( !visibility->val( i ) && activeCellInfo->isActive( i ) )
        {
            size_t cellResultIndex        = activeCellInfo->cellResultIndex( i );
            resultValues[cellResultIndex] = inputValues[cellResultIndex];
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculation::replaceFilteredValuesWithDefaultValue( double                        defaultValue,
                                                                cvf::ref<cvf::UByteArray>     visibility,
                                                                std::vector<double>&          resultValues,
                                                                RiaDefines::PorosityModelType porosityModel,
                                                                RimEclipseCase*               outputEclipseCase )

{
    auto activeCellInfo = outputEclipseCase->eclipseCaseData()->activeCellInfo( porosityModel );
    int  numCells       = static_cast<int>( visibility->size() );

#pragma omp parallel for
    for ( int i = 0; i < numCells; i++ )
    {
        if ( !visibility->val( i ) && activeCellInfo->isActive( i ) )
        {
            size_t cellResultIndex        = activeCellInfo->cellResultIndex( i );
            resultValues[cellResultIndex] = defaultValue;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculation::filterResults( RimGridView*                            cellFilterView,
                                        const std::vector<std::vector<double>>& values,
                                        RimGridCalculation::DefaultValueType    defaultValueType,
                                        double                                  defaultValue,
                                        std::vector<double>&                    resultValues,
                                        RiaDefines::PorosityModelType           porosityModel,
                                        RimEclipseCase*                         outputEclipseCase ) const

{
    auto visibility = cellFilterView->currentTotalCellVisibility();

    if ( defaultValueType == RimGridCalculation::DefaultValueType::FROM_PROPERTY )
    {
        if ( m_defaultPropertyVariableIndex < static_cast<int>( values.size() ) )
            replaceFilteredValuesWithVector( values[m_defaultPropertyVariableIndex], visibility, resultValues, porosityModel, outputEclipseCase );
        else
        {
            QString errorMessage = "Invalid input data for default result property, no data assigned to non-visible cells.";
            RiaLogging::errorInMessageBox( nullptr, "Grid Property Calculator", errorMessage );
        }
    }
    else
    {
        double valueToUse = defaultValue;
        if ( defaultValueType == RimGridCalculation::DefaultValueType::POSITIVE_INFINITY ) valueToUse = HUGE_VAL;

        replaceFilteredValuesWithDefaultValue( valueToUse, visibility, resultValues, porosityModel, outputEclipseCase );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculation::updateDependentObjects()
{
    for ( RimEclipseCase* eclipseCase : outputEclipseCases() )
    {
        if ( eclipseCase )
        {
            RimReloadCaseTools::updateAll3dViews( eclipseCase );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculation::removeDependentObjects()
{
    QString leftHandSideVariableName = RimGridCalculation::findLeftHandSide( m_expression );

    auto porosityModel = RiaDefines::PorosityModelType::MATRIX_MODEL;

    RigEclipseResultAddress resAddr( RiaDefines::ResultCatType::GENERATED, leftHandSideVariableName );

    for ( RimEclipseCase* eclipseCase : outputEclipseCases() )
    {
        if ( eclipseCase )
        {
            // Select "None" result if the result that is being removed were displayed in a view.
            for ( auto v : eclipseCase->reservoirViews() )
            {
                if ( v->cellResult()->resultType() == resAddr.resultCatType() && v->cellResult()->resultVariable() == resAddr.resultName() )
                {
                    v->cellResult()->setResultType( RiaDefines::ResultCatType::GENERATED );
                    v->cellResult()->setResultVariable( "None" );
                }
            }

            eclipseCase->results( porosityModel )->clearScalarResult( resAddr );
            eclipseCase->results( porosityModel )->setRemovedTagOnGeneratedResult( resAddr );

            RimReloadCaseTools::updateAll3dViews( eclipseCase );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCalculation::calculateForCases( const std::vector<RimEclipseCase*>& calculationCases, std::optional<std::vector<size_t>> timeSteps )
{
    if ( calculationCases.empty() ) return true;

    QString leftHandSideVariableName = RimGridCalculation::findLeftHandSide( m_expression );

    auto [isOk, errorMessage] = validateVariables();
    if ( !isOk )
    {
        RiaLogging::errorInMessageBox( nullptr, "Grid Property Calculator", errorMessage );
        return false;
    }

    for ( auto calculationCase : calculationCases )
    {
        for ( auto inputCase : inputCases() )
        {
            if ( !calculationCase->isGridSizeEqualTo( inputCase ) )
            {
                QString msg = "Detected IJK mismatch between input cases and destination case. All grid "
                              "cases must have identical IJK sizes.";
                RiaLogging::errorInMessageBox( nullptr, "Grid Property Calculator", msg );
                return false;
            }
        }
    }

    const bool isMultipleCasesPresent = calculationCases.size() > 1;

    if ( isMultipleCasesPresent )
    {
        QString txt = "Starting calculation " + description() + " for " + QString::number( calculationCases.size() ) + " cases.";
        RiaLogging::info( txt );
    }

    caf::ProgressInfo progressInfo( calculationCases.size(), "Processing Grid Calculations" );
    size_t            progressIndex = 0;

    bool anyErrorsDetected = false;
    for ( RimEclipseCase* calculationCase : calculationCases )
    {
        if ( !calculationCase )
        {
            RiaLogging::errorInMessageBox( nullptr,
                                           "Grid Property Calculator",
                                           QString( "No case found for calculation : %1" ).arg( leftHandSideVariableName ) );
            anyErrorsDetected = true;
            continue;
        }

        auto porosityModel = RiaDefines::PorosityModelType::MATRIX_MODEL;

        RigEclipseResultAddress resAddr( RiaDefines::ResultCatType::GENERATED, leftHandSideVariableName );

        if ( !calculationCase->results( porosityModel )->ensureKnownResultLoaded( resAddr ) )
        {
            bool needsToBeStored = false;
            calculationCase->results( porosityModel )->createResultEntry( resAddr, needsToBeStored );
        }

        calculationCase->results( porosityModel )->clearScalarResult( resAddr );

        // If an input grid is present, max time step count is zero. Make sure the time step count for the calculation is
        // always 1 or more.
        const size_t timeStepCount = std::max( size_t( 1 ), calculationCase->results( porosityModel )->maxTimeStepCount() );

        std::vector<std::vector<double>>* scalarResultFrames =
            calculationCase->results( porosityModel )->modifiableCellScalarResultTimesteps( resAddr );
        scalarResultFrames->resize( timeStepCount );

        for ( size_t tsId = 0; tsId < timeStepCount; tsId++ )
        {
            // Skip time steps that are not in the list of time steps to calculate
            if ( timeSteps && std::find( timeSteps->begin(), timeSteps->end(), tsId ) == timeSteps->end() ) continue;

            std::vector<std::vector<double>> values;
            for ( size_t i = 0; i < m_variables.size(); i++ )
            {
                RimGridCalculationVariable* v = dynamic_cast<RimGridCalculationVariable*>( m_variables[i] );
                CAF_ASSERT( v != nullptr );
                values.push_back( getInputVectorForVariable( v, tsId, porosityModel, calculationCase ) );
            }

            ExpressionParser parser;
            for ( size_t i = 0; i < m_variables.size(); i++ )
            {
                RimGridCalculationVariable* v = dynamic_cast<RimGridCalculationVariable*>( m_variables[i] );
                CAF_ASSERT( v != nullptr );
                parser.assignVector( v->name(), values[i] );
            }

            std::vector<double> resultValues;
            resultValues.resize( values[0].size() );
            parser.assignVector( leftHandSideVariableName, resultValues );

            QString errorText;
            bool    evaluatedOk = parser.expandIfStatementsAndEvaluate( m_expression, &errorText );

            if ( evaluatedOk )
            {
                if ( m_cellFilterView() )
                {
                    filterResults( m_cellFilterView(), values, m_defaultValueType(), m_defaultValue(), resultValues, porosityModel, calculationCase );
                }

                scalarResultFrames->at( tsId ) = resultValues;

                m_isDirty = false;
            }
            else
            {
                QString s = "The following error message was received from the parser library : \n\n";
                s += errorText;

                RiaLogging::errorInMessageBox( nullptr, "Grid Property Calculator", s );
                return false;
            }

            calculationCase->updateResultAddressCollection();
        }

        if ( isMultipleCasesPresent )
        {
            QString txt = "    " + calculationCase->caseUserDescription();
            RiaLogging::info( txt );
        }

        progressInfo.setProgress( progressIndex++ );
    }

    if ( isMultipleCasesPresent )
    {
        QString txt = "Completed calculation " + description() + " for " + QString::number( outputEclipseCases().size() ) + " cases";
        RiaLogging::info( txt );
    }

    return !anyErrorsDetected;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, QString> RimGridCalculation::validateVariables()
{
    auto porosityModel = RiaDefines::PorosityModelType::MATRIX_MODEL;

    for ( size_t i = 0; i < m_variables.size(); i++ )
    {
        RimGridCalculationVariable* v = dynamic_cast<RimGridCalculationVariable*>( m_variables[i] );
        CAF_ASSERT( v != nullptr );

        if ( !v->eclipseCase() )
        {
            QString errorMessage = QString( "No case defined for variable : %1" ).arg( v->name() );
            return std::make_pair( false, errorMessage );
        }

        if ( v->resultVariable().isEmpty() )
        {
            QString errorMessage = QString( "No result variable defined for variable : %1" ).arg( v->name() );
            return std::make_pair( false, errorMessage );
        }

        auto                    resultCategoryType = v->resultCategoryType();
        RigEclipseResultAddress resAddr( resultCategoryType, v->resultVariable() );
        if ( !v->eclipseCase()->results( porosityModel )->ensureKnownResultLoaded( resAddr ) )
        {
            QString errorMessage = QString( "Unable to load result for variable : %1" ).arg( v->name() );
            return std::make_pair( false, errorMessage );
        }
    }

    return std::make_pair( true, "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculation::onChildrenUpdated( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& updatedObjects )
{
    if ( childArray == &m_variables )
    {
        // Update the editors of all the variables if a variable changes.
        // This makes the read-only state of the filter parameters consistent:
        // only one filter is allowed at a time.
        for ( auto v : m_variables )
        {
            v->updateConnectedEditors();
        }
    }
}
