/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RicExportFractureCompletionsImpl.h"

#include "RicWellPathFractureReportItem.h"

#include "RiaEclipseUnitTools.h"
#include "RiaLogging.h"
#include "RiaQDateTimeTools.h"
#include "RiaSummaryTools.h"
#include "RigEclipseResultAddress.h"

#include "RimEclipseCase.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimFracture.h"
#include "RimFractureContainmentTools.h"
#include "RimFractureTemplate.h"
#include "RimObservedEclipseUserData.h"
#include "RimProject.h"
#include "RimReloadCaseTools.h"
#include "RimSimWellFracture.h"
#include "RimSimWellFractureCollection.h"
#include "RimSimWellInView.h"
#include "RimStimPlanFractureTemplate.h"
#include "RimSummaryCase.h"
#include "RimWellPath.h"
#include "RimWellPathCompletions.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"

#include "RifEclipseSummaryAddress.h"
#include "RifSummaryReaderInterface.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseToStimPlanCalculator.h"
#include "RigEclipseToStimPlanCellTransmissibilityCalculator.h"
#include "RigFractureCell.h"
#include "RigFractureGrid.h"
#include "RigFractureTransmissibilityEquations.h"
#include "RigMainGrid.h"
#include "RigResultAccessorFactory.h"
#include "RigSimWellData.h"
#include "RigSimulationWellCoordsAndMD.h"
#include "RigTransmissibilityCondenser.h"
#include "RigTransmissibilityEquations.h"
#include "RigWellPath.h"
#include "RigWellPathStimplanIntersector.h"

#include "cvfGeometryTools.h"
#include "cvfPlane.h"

#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData>
    RicExportFractureCompletionsImpl::generateCompdatValuesForWellPath( RimWellPath*                                wellPath,
                                                                        RimEclipseCase*                             caseToApply,
                                                                        std::vector<RicWellPathFractureReportItem>* fractureDataForReport,
                                                                        QTextStream*                outputStreamForIntermediateResultsText,
                                                                        PressureDepletionParameters pdParams )
{
    std::vector<const RimFracture*> fracturesAlongWellPath;

    for ( auto& frac : wellPath->fractureCollection()->activeFractures() )
    {
        frac->ensureValidNonDarcyProperties();

        fracturesAlongWellPath.push_back( frac );
    }

    return generateCompdatValues( caseToApply,
                                  wellPath->completionSettings()->wellNameForExport(),
                                  wellPath->wellPathGeometry(),
                                  fracturesAlongWellPath,
                                  fractureDataForReport,
                                  outputStreamForIntermediateResultsText,
                                  pdParams );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData>
    RicExportFractureCompletionsImpl::generateCompdatValuesForSimWell( RimEclipseCase*             eclipseCase,
                                                                       const RimSimWellInView*     well,
                                                                       QTextStream*                outputStreamForIntermediateResultsText,
                                                                       PressureDepletionParameters pdParams )
{
    std::vector<RigCompletionData> completionData;

    auto branches = well->wellPipeBranches();

    for ( size_t branchIndex = 0; branchIndex < branches.size(); ++branchIndex )
    {
        std::vector<const RimFracture*> fractures;
        for ( RimSimWellFracture* fracture : well->simwellFractureCollection->simwellFractures() )
        {
            if ( fracture->isChecked() && static_cast<size_t>( fracture->branchIndex() ) == branchIndex )
            {
                fracture->ensureValidNonDarcyProperties();

                fractures.push_back( fracture );
            }
        }

        std::vector<RigCompletionData> branchCompletions = generateCompdatValues( eclipseCase,
                                                                                  well->name(),
                                                                                  branches[branchIndex],
                                                                                  fractures,
                                                                                  nullptr,
                                                                                  outputStreamForIntermediateResultsText,
                                                                                  pdParams );

        completionData.insert( completionData.end(), branchCompletions.begin(), branchCompletions.end() );
    }

    return completionData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData>
    RicExportFractureCompletionsImpl::generateCompdatValues( RimEclipseCase*                             caseToApply,
                                                             const QString&                              wellNameForExport,
                                                             const RigWellPath*                          wellPathGeometry,
                                                             const std::vector<const RimFracture*>&      fractures,
                                                             std::vector<RicWellPathFractureReportItem>* fractureDataReportItems,
                                                             QTextStream*                outputStreamForIntermediateResultsText,
                                                             PressureDepletionParameters pdParams )
{
    std::vector<RigCompletionData> fractureCompletions;

    if ( !caseToApply || !caseToApply->eclipseCaseData() )
    {
        return fractureCompletions;
    }

    auto cellResultsData = caseToApply->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    if ( !cellResultsData )
    {
        return fractureCompletions;
    }

    {
        // Load the data required by computations to be able to use const access only inside OpenMP loop

        std::vector<QString> resultNames = RigEclipseToStimPlanCellTransmissibilityCalculator::requiredResultNames();

        bool loadingSucceeded = RicExportFractureCompletionsImpl::loadResultsByName( cellResultsData, resultNames );
        if ( !loadingSucceeded )
        {
            QString msg;
            msg += "Compdat Export : One or more of the following required data sources are missing :";

            for ( const auto& r : resultNames )
            {
                msg += " ";
                msg += r;
            }
            RiaLogging::error( msg );

            return fractureCompletions;
        }
    }

    {
        // Load the data required by fracture summary header

        std::vector<QString> resultNames{ "TRANX", "TRANY", "TRANZ" };
        RicExportFractureCompletionsImpl::loadResultsByName( cellResultsData, resultNames );
    }

    {
        // Optional results
        std::vector<QString> resultNames = RigEclipseToStimPlanCellTransmissibilityCalculator::optionalResultNames();
        RicExportFractureCompletionsImpl::loadResultsByName( cellResultsData, resultNames );
    }

    if ( pdParams.performScaling )
    {
        RigCaseCellResultsData* results = caseToApply->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
        results->ensureKnownResultLoaded( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "PRESSURE" ) );
    }

    return generateCompdatValuesConst( caseToApply,
                                       wellNameForExport,
                                       wellPathGeometry,
                                       fractures,
                                       fractureDataReportItems,
                                       outputStreamForIntermediateResultsText,
                                       pdParams );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData>
    RicExportFractureCompletionsImpl::generateCompdatValuesConst( const RimEclipseCase*                       caseToApply,
                                                                  const QString&                              wellNameForExport,
                                                                  const RigWellPath*                          wellPathGeometry,
                                                                  const std::vector<const RimFracture*>&      fractures,
                                                                  std::vector<RicWellPathFractureReportItem>* fractureDataReportItems,
                                                                  QTextStream*                outputStreamForIntermediateResultsText,
                                                                  PressureDepletionParameters pdParams )
{
    std::vector<RigCompletionData> fractureCompletions;

    if ( !caseToApply || !caseToApply->eclipseCaseData() || !wellPathGeometry )
    {
        return fractureCompletions;
    }

    double             cDarcyInCorrectUnit = RiaEclipseUnitTools::darcysConstant( caseToApply->eclipseCaseData()->unitsType() );
    const RigMainGrid* mainGrid            = caseToApply->eclipseCaseData()->mainGrid();

    const RigCaseCellResultsData* results = caseToApply->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    const RigActiveCellInfo* actCellInfo  = caseToApply->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

    bool performPressureDepletionScaling = pdParams.performScaling;

    int    initialWellProductionTimeStep = 0;
    double currentWellPressure           = 0;

    if ( performPressureDepletionScaling )
    {
        double userWBHP = pdParams.userWBHP;

        double initialWBHPFromSummary = 0.0;
        double currentWBHPFromSummary = 0.0;
        // Find well pressures (WBHP) from summary case.
        getWellPressuresAndInitialProductionTimeStepFromSummaryData( caseToApply,
                                                                     wellNameForExport,
                                                                     pdParams.pressureScalingTimeStep,
                                                                     &initialWellProductionTimeStep,
                                                                     &initialWBHPFromSummary,
                                                                     &currentWBHPFromSummary );

        if ( pdParams.wbhpSource == WBHP_FROM_SUMMARY )
        {
            currentWellPressure = currentWBHPFromSummary;
            if ( pdParams.pressureScalingTimeStep <= initialWellProductionTimeStep )
            {
                currentWellPressure = userWBHP;
            }
        }
        else
        {
            currentWellPressure = userWBHP;
        }
    }

    const std::vector<std::vector<double>>* pressureResultVector   = nullptr;
    const std::vector<double>*              currentMatrixPressures = nullptr;
    if ( performPressureDepletionScaling )
    {
        pressureResultVector = &results->cellScalarResults( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "PRESSURE" ) );
        CVF_ASSERT( !pressureResultVector->empty() );

        if ( pdParams.pressureScalingTimeStep < static_cast<int>( pressureResultVector->size() ) )
        {
            currentMatrixPressures = &pressureResultVector->at( pdParams.pressureScalingTimeStep );
        }
        else
        {
            // Don't perform scaling if the current pressure time step is beyond the case range.
            performPressureDepletionScaling = false;
        }
    }

    // To handle several fractures in the same eclipse cell we need to keep track of the transmissibility
    // to the well from each fracture intersecting the cell and sum these transmissibilities at the end.
    // std::map <eclipseCellIndex ,map< fracture, trans> >
    // std::map<size_t, std::map<const RimFracture*, double>> eclCellIdxToTransPrFractureMap;

    std::vector<std::vector<RigCompletionData>> sharedComplForFracture( fractures.size() );

    const double transmissibilityThreshold = 1e-9;

#pragma omp parallel for
    for ( int i = 0; i < (int)fractures.size(); i++ )
    {
        const RimFracture*         fracture     = fractures[i];
        const RimFractureTemplate* fracTemplate = fracture->fractureTemplate();

        if ( !fracTemplate ) continue;

        const RigFractureGrid* fractureGrid = fracture->fractureGrid();
        if ( !fractureGrid ) continue;

        bool useFiniteConductivityInFracture = fracTemplate->useFiniteConductivityInFracture();

        // If finite cond chosen and conductivity not present in stimplan file, do not calculate trans for this fracture
        if ( useFiniteConductivityInFracture && !checkForStimPlanConductivity( fracTemplate, fracture ) )
        {
            continue;
        }

        RigTransmissibilityCondenser transCondenser;
        transCondenser.setTransmissibilityThreshold( transmissibilityThreshold );

        //////
        // Calculate Matrix To Fracture Trans
        RigEclipseToStimPlanCalculator eclToFractureCalc( caseToApply,
                                                          fracture->transformMatrix(),
                                                          fracTemplate->skinFactor(),
                                                          cDarcyInCorrectUnit,
                                                          *fractureGrid,
                                                          fracture );

        eclToFractureCalc.appendDataToTransmissibilityCondenser( useFiniteConductivityInFracture, &transCondenser );

        if ( useFiniteConductivityInFracture )
        {
            calculateInternalFractureTransmissibilities( fractureGrid, cDarcyInCorrectUnit, transCondenser );
            bool useInfiniteWellPI = fracTemplate->conductivityType() == RimFractureTemplate::FINITE_CONDUCTIVITY_INFINITE_WELL_PI;
            calculateFractureToWellTransmissibilities( fracTemplate,
                                                       fractureGrid,
                                                       fracture,
                                                       cDarcyInCorrectUnit,
                                                       wellPathGeometry,
                                                       transCondenser,
                                                       useInfiniteWellPI );
        }

        /////
        // Insert total transmissibility from eclipse-cell to well for this fracture into the map
        std::map<size_t, double> matrixToWellTrans = calculateMatrixToWellTransmissibilities( transCondenser );

        double maxPressureDrop = 0.0, minPressureDrop = 0.0;
        if ( performPressureDepletionScaling )
        {
            RigTransmissibilityCondenser scaledCondenser = transCondenser;
            // 1. Scale matrix to fracture transmissibilities by matrix to fracture pressure
            std::map<size_t, double> originalLumpedMatrixToFractureTrans =
                scaledCondenser.scaleMatrixToFracTransByMatrixWellDP( actCellInfo,
                                                                      currentWellPressure,
                                                                      *currentMatrixPressures,
                                                                      &minPressureDrop,
                                                                      &maxPressureDrop );
            // 2: Calculate new external transmissibilities
            scaledCondenser.calculateCondensedTransmissibilities();

            { // 3: H�gst�l correction.

                // a. Calculate new effective fracture to well transmissiblities
                std::map<size_t, double> fictitiousFractureToWellTransmissibilities =
                    scaledCondenser.calculateFicticiousFractureToWellTransmissibilities();
                // b. Calculate new effective matrix to well transmissibilities
                std::map<size_t, double> effectiveMatrixToWellTrans =
                    scaledCondenser.calculateEffectiveMatrixToWellTransmissibilities( originalLumpedMatrixToFractureTrans,
                                                                                      fictitiousFractureToWellTransmissibilities );
                matrixToWellTrans = effectiveMatrixToWellTrans;
            }
        }

        std::vector<RigCompletionData> allCompletionsForOneFracture =
            generateCompdatValuesForFracture( matrixToWellTrans, wellNameForExport, caseToApply, fracture, fracTemplate );

        if ( fractureDataReportItems )
        {
            RicWellPathFractureReportItem reportItem( wellNameForExport, fracture->name(), fracTemplate->name() );
            reportItem.setUnitSystem( fracTemplate->fractureTemplateUnit() );

            if ( performPressureDepletionScaling )
            {
                QString timeStepString;
                if ( pdParams.pressureScalingTimeStep < caseToApply->timeStepStrings().size() )
                {
                    timeStepString = caseToApply->timeStepStrings()[pdParams.pressureScalingTimeStep];
                }
                reportItem.setPressureDepletionParameters( performPressureDepletionScaling,
                                                           timeStepString,
                                                           caf::AppEnum<PressureDepletionWBHPSource>::uiTextFromIndex( pdParams.wbhpSource ),
                                                           pdParams.userWBHP,
                                                           currentWellPressure,
                                                           minPressureDrop,
                                                           maxPressureDrop );
            }

            RicExportFractureCompletionsImpl::calculateAndSetReportItemData( allCompletionsForOneFracture, eclToFractureCalc, reportItem );

#pragma omp critical( critical_section_fractureDataReportItems )
            {
                fractureDataReportItems->push_back( reportItem );
            }
        }

        std::copy( allCompletionsForOneFracture.begin(), allCompletionsForOneFracture.end(), std::back_inserter( sharedComplForFracture[i] ) );

        if ( outputStreamForIntermediateResultsText )
        {
#pragma omp critical( critical_section_outputStreamForIntermediateResultsText )
            {
                outputIntermediateResultsText( outputStreamForIntermediateResultsText, fracture, transCondenser, mainGrid, fractureGrid );
            }
        }
    }

    for ( const auto& completions : sharedComplForFracture )
    {
        std::copy( completions.begin(), completions.end(), std::back_inserter( fractureCompletions ) );
    }
    return fractureCompletions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportFractureCompletionsImpl::getWellPressuresAndInitialProductionTimeStepFromSummaryData( const RimEclipseCase* caseToApply,
                                                                                                    const QString&        wellPathName,
                                                                                                    int                   currentTimeStep,
                                                                                                    int*    initialCaseTimeStep,
                                                                                                    double* initialWellPressure,
                                                                                                    double* currentWellPressure )
{
    const RimEclipseResultCase* resultCase = dynamic_cast<const RimEclipseResultCase*>( caseToApply );
    if ( resultCase )
    {
        std::vector<QDateTime> caseTimeSteps = resultCase->timeStepDates();
        if ( caseTimeSteps.empty() ) return;

        QDateTime initialProductionDate;
        QDateTime currentDate;
        if ( currentTimeStep < static_cast<int>( caseTimeSteps.size() ) )
        {
            currentDate = caseTimeSteps[currentTimeStep];
        }
        else
        {
            currentDate = caseTimeSteps.back();
        }

        RifEclipseSummaryAddress wbhpPressureAddress = RifEclipseSummaryAddress::wellAddress( "WBHP", wellPathName.toStdString() );

        auto summaryCase = RimReloadCaseTools::findSummaryCaseFromEclipseResultCase( resultCase );
        if ( summaryCase && summaryCase->summaryReader() )
        {
            auto [isOk, values] = summaryCase->summaryReader()->values( wbhpPressureAddress );
            if ( isOk )
            {
                std::vector<time_t> summaryTimeSteps = summaryCase->summaryReader()->timeSteps( wbhpPressureAddress );
                CVF_ASSERT( values.size() == summaryTimeSteps.size() );
                for ( size_t i = 0; i < summaryTimeSteps.size(); ++i )
                {
                    QDateTime summaryDate = RiaQDateTimeTools::fromTime_t( summaryTimeSteps[i] );
                    if ( initialProductionDate.isNull() && values[i] > 0.0 )
                    {
                        initialProductionDate = summaryDate;
                        *initialWellPressure  = values[i];
                    }
                    if ( summaryDate <= currentDate )
                    {
                        *currentWellPressure = values[i];
                    }
                }
            }
        }
        if ( initialProductionDate.isValid() )
        {
            for ( size_t i = 0; i < caseTimeSteps.size(); ++i )
            {
                // Pick last time step that isn't bigger than the initial production time.
                if ( caseTimeSteps[i] < initialProductionDate )
                {
                    *initialCaseTimeStep = static_cast<int>( i );
                }
                else
                {
                    break;
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportFractureCompletionsImpl::checkForStimPlanConductivity( const RimFractureTemplate* fracTemplate, const RimFracture* fracture )
{
    auto fracTemplateStimPlan = dynamic_cast<const RimMeshFractureTemplate*>( fracTemplate );
    if ( fracTemplateStimPlan )
    {
        if ( !fracTemplateStimPlan->hasConductivity() )
        {
            RiaLogging::error( "Trying to export completion data for stimPlan fracture without conductivity data for " + fracture->name() );
            RiaLogging::error( "No transmissibilities will be calculated for " + fracture->name() );
            return false;
        }
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportFractureCompletionsImpl::calculateInternalFractureTransmissibilities( const RigFractureGrid*        fractureGrid,
                                                                                    double                        cDarcyInCorrectUnit,
                                                                                    RigTransmissibilityCondenser& transCondenser )
{
    for ( size_t i = 0; i < fractureGrid->iCellCount(); i++ )
    {
        for ( size_t j = 0; j < fractureGrid->jCellCount(); j++ )
        {
            size_t fractureCellIndex = fractureGrid->getGlobalIndexFromIJ( i, j );

            const RigFractureCell& fractureCell = fractureGrid->cellFromIndex( fractureCellIndex );

            if ( !fractureCell.hasNonZeroConductivity() ) continue;

            if ( i < fractureGrid->iCellCount() - 1 )
            {
                size_t                 fractureCellNeighbourXIndex = fractureGrid->getGlobalIndexFromIJ( i + 1, j );
                const RigFractureCell& fractureCellNeighbourX      = fractureGrid->cellFromIndex( fractureCellNeighbourXIndex );

                double horizontalTransToXneigbour =
                    RigFractureTransmissibilityEquations::centerToCenterFractureCellTrans( fractureCell.getConductivityValue(),
                                                                                           fractureCell.cellSizeX(),
                                                                                           fractureCell.cellSizeZ(),
                                                                                           fractureCellNeighbourX.getConductivityValue(),
                                                                                           fractureCellNeighbourX.cellSizeX(),
                                                                                           fractureCellNeighbourX.cellSizeZ(),
                                                                                           cDarcyInCorrectUnit );

                transCondenser.addNeighborTransmissibility( { false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, fractureCellIndex },
                                                            { false,
                                                              RigTransmissibilityCondenser::CellAddress::STIMPLAN,
                                                              fractureCellNeighbourXIndex },
                                                            horizontalTransToXneigbour );
            }

            if ( j < fractureGrid->jCellCount() - 1 )
            {
                size_t                 fractureCellNeighbourZIndex = fractureGrid->getGlobalIndexFromIJ( i, j + 1 );
                const RigFractureCell& fractureCellNeighbourZ      = fractureGrid->cellFromIndex( fractureCellNeighbourZIndex );

                double verticalTransToZneigbour =
                    RigFractureTransmissibilityEquations::centerToCenterFractureCellTrans( fractureCell.getConductivityValue(),
                                                                                           fractureCell.cellSizeZ(),
                                                                                           fractureCell.cellSizeX(),
                                                                                           fractureCellNeighbourZ.getConductivityValue(),
                                                                                           fractureCellNeighbourZ.cellSizeZ(),
                                                                                           fractureCellNeighbourZ.cellSizeX(),
                                                                                           cDarcyInCorrectUnit );

                transCondenser.addNeighborTransmissibility( { false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, fractureCellIndex },
                                                            { false,
                                                              RigTransmissibilityCondenser::CellAddress::STIMPLAN,
                                                              fractureCellNeighbourZIndex },
                                                            verticalTransToZneigbour );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportFractureCompletionsImpl::calculateFractureToWellTransmissibilities( gsl::not_null<const RimFractureTemplate*> fracTemplate,
                                                                                  gsl::not_null<const RigFractureGrid*>     fractureGrid,
                                                                                  gsl::not_null<const RimFracture*>         fracture,
                                                                                  double                            cDarcyInCorrectUnit,
                                                                                  gsl::not_null<const RigWellPath*> wellPathGeometry,
                                                                                  RigTransmissibilityCondenser&     transCondenser,
                                                                                  bool                              useInfiniteWellPI )
{
    // If fracture has orientation Azimuth (without user-defined perforation length) or Transverse,
    // assume only radial inflow
    bool useRadialInflow =
        ( fracTemplate->orientationType() == RimFractureTemplate::AZIMUTH && !fracTemplate->useUserDefinedPerforationLength() ) ||
        fracTemplate->orientationType() == RimFractureTemplate::TRANSVERSE_WELL_PATH;

    const double infiniteWellTrans = 1000000.0;

    if ( useRadialInflow )
    {
        std::pair<size_t, size_t> wellCellIJ    = fractureGrid->fractureCellAtWellCenter();
        size_t                    wellCellIndex = fractureGrid->getGlobalIndexFromIJ( wellCellIJ.first, wellCellIJ.second );

        const RigFractureCell& wellCell = fractureGrid->cellFromIndex( wellCellIndex );

        double radialTrans = RigFractureTransmissibilityEquations::fractureCellToWellRadialTrans( wellCell.getConductivityValue(),
                                                                                                  wellCell.cellSizeX(),
                                                                                                  wellCell.cellSizeZ(),
                                                                                                  fracture->wellRadius(),
                                                                                                  fracTemplate->skinFactor(),
                                                                                                  cDarcyInCorrectUnit );

        if ( useInfiniteWellPI )
        {
            radialTrans = infiniteWellTrans;
        }

        transCondenser.addNeighborTransmissibility( { true, RigTransmissibilityCondenser::CellAddress::WELL, 1 },
                                                    { false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, wellCellIndex },
                                                    radialTrans );
    }
    else
    {
        std::vector<cvf::Vec3d> wellPathPoints;

        if ( fracTemplate->orientationType() == RimFractureTemplate::ALONG_WELL_PATH )
        {
            // If fracture has orientation along well, linear inflow along well and radial flow at endpoints
            wellPathPoints = wellPathGeometry->wellPathPointsIncludingInterpolatedIntersectionPoint( fracture->fractureMD() );
        }
        else
        {
            // If fracture has azimuth orientation with user-defined perforation length use linear inflow along
            // well and radial flow at endpoints on a well path which is rotated into the fracture plane.
            CAF_ASSERT( fracTemplate->orientationType() == RimFractureTemplate::AZIMUTH );
            CAF_ASSERT( fracTemplate->useUserDefinedPerforationLength() );

            wellPathPoints = computeWellPointsInFracturePlane( fracture, wellPathGeometry );
        }

        RigWellPathStimplanIntersector                                                wellFractureIntersector( wellPathPoints, fracture );
        const std::map<size_t, RigWellPathStimplanIntersector::WellCellIntersection>& fractureWellCells =
            wellFractureIntersector.intersections();

        for ( const auto& fracCellIdxIsectDataPair : fractureWellCells )
        {
            size_t fracWellCellIdx = fracCellIdxIsectDataPair.first;

            RigWellPathStimplanIntersector::WellCellIntersection intersection = fracCellIdxIsectDataPair.second;

            const RigFractureCell& fractureWellCell = fractureGrid->cellFromIndex( fracWellCellIdx );

            double linearTrans = 0.0;
            if ( ( intersection.hlength > 0.0 || intersection.vlength > 0.0 ) && !std::isinf( fractureWellCell.getConductivityValue() ) )
            {
                linearTrans = RigFractureTransmissibilityEquations::fractureCellToWellLinearTrans( fractureWellCell.getConductivityValue(),
                                                                                                   fractureWellCell.cellSizeX(),
                                                                                                   fractureWellCell.cellSizeZ(),
                                                                                                   intersection.vlength,
                                                                                                   intersection.hlength,
                                                                                                   fracture->perforationEfficiency(),
                                                                                                   fracTemplate->skinFactor(),
                                                                                                   cDarcyInCorrectUnit,
                                                                                                   fracture->wellRadius() );
            }

            if ( useInfiniteWellPI && linearTrans > 0.0 )
            {
                linearTrans = infiniteWellTrans;
            }

            transCondenser.addNeighborTransmissibility( { true, RigTransmissibilityCondenser::CellAddress::WELL, 1 },
                                                        { false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, fracWellCellIdx },
                                                        linearTrans );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RicExportFractureCompletionsImpl::computeWellPointsInFracturePlane( gsl::not_null<const RimFracture*> fracture,
                                                                                            gsl::not_null<const RigWellPath*> wellPathGeometry )

{
    std::vector<cvf::Vec3d> wellPathPoints = wellPathGeometry->wellPathPointsIncludingInterpolatedIntersectionPoint( fracture->fractureMD() );

    RiaLogging::info( "Using user-defined perforation length on azimuth fracture." );
    RiaLogging::info( QString( "Perforation length: %1" ).arg( fracture->perforationLength() ) );
    double startMd = fracture->fractureMD() - fracture->perforationLength() / 2.0;
    double endMd   = fracture->fractureMD() + fracture->perforationLength() / 2.0;

    cvf::Vec3d anchorPosition = wellPathGeometry->interpolatedPointAlongWellPath( fracture->fractureMD() );

    auto coordsAndMd = wellPathGeometry->clippedPointSubset( startMd, endMd );

    auto       wellPathCoords = coordsAndMd.first;
    cvf::Vec3d startPos       = wellPathCoords.front();
    cvf::Vec3d endPos         = wellPathCoords.back();

    cvf::Plane fracturePlane;
    auto       fractureTransform = fracture->transformMatrix();
    fracturePlane.setFromPointAndNormal( fractureTransform.translation(), static_cast<cvf::Vec3d>( fractureTransform.col( 2 ) ) );

    // Get the direction from the start position to the fracture plane
    cvf::Vec3d startPosInFracturePlane            = fracturePlane.projectPoint( startPos );
    cvf::Vec3d directionToStartPosInFracturePlane = ( startPosInFracturePlane - anchorPosition ).getNormalized();
    cvf::Vec3d directionToStartPos                = ( startPos - anchorPosition ).getNormalized();

    auto vecToString = []( const cvf::Vec3d& vec ) { return QString( "[%1 %2 %3]" ).arg( vec.x() ).arg( vec.y() ).arg( vec.z() ); };

    RiaLogging::info( QString( "Start perforation position: %1" ).arg( vecToString( startPos ) ) );
    RiaLogging::info( QString( "Start perforation position in fracture plane: %1" ).arg( vecToString( startPosInFracturePlane ) ) );
    RiaLogging::info( QString( "Anchor pos: %1" ).arg( vecToString( anchorPosition ) ) );
    RiaLogging::info( QString( "Direction anchor to start position in plane: %1" ).arg( vecToString( directionToStartPosInFracturePlane ) ) );
    RiaLogging::info( QString( "Direction anchor to original start position: %1" ).arg( vecToString( directionToStartPos ) ) );

    // Find the angle between the real direction (tangential to well path) and the fracture plane
    double angle = cvf::GeometryTools::getAngle( directionToStartPosInFracturePlane, directionToStartPos );
    RiaLogging::info( QString( "Angle: %1 degrees." ).arg( cvf::Math::toDegrees( angle ) ) );
    auto rotMat = cvf::GeometryTools::rotationMatrixBetweenVectors( directionToStartPos, directionToStartPosInFracturePlane );

    auto rotatePoint = []( const cvf::Vec3d& point, const cvf::Vec3d& offset, auto rotMat )
    {
        cvf::Vec3d p = point - offset;
        p.transformPoint( rotMat );
        p += offset;
        return p;
    };

    // Rotate the well path points into the plane
    for ( auto& r : wellPathPoints )
    {
        r = rotatePoint( r, anchorPosition, rotMat );
    }

    cvf::Vec3d transformedStartPos = rotatePoint( startPos, anchorPosition, rotMat );
    RiaLogging::info( QString( "Perforation start position: original: %1 --> adapted: %2" )
                          .arg( vecToString( startPos ) )
                          .arg( vecToString( transformedStartPos ) ) );

    cvf::Vec3d transformedEndPos = rotatePoint( endPos, anchorPosition, rotMat );
    RiaLogging::info(
        QString( "Perforation end position: original: %1 --> adapted: %2" ).arg( vecToString( endPos ) ).arg( vecToString( transformedEndPos ) ) );

    return wellPathPoints;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<size_t, double> RicExportFractureCompletionsImpl::calculateMatrixToWellTransmissibilities( RigTransmissibilityCondenser& transCondenser )
{
    std::map<size_t, double> matrixToWellTransmissibilities;

    std::set<RigTransmissibilityCondenser::CellAddress> externalCells = transCondenser.externalCells();
    for ( RigTransmissibilityCondenser::CellAddress externalCell : externalCells )
    {
        if ( externalCell.m_cellIndexSpace == RigTransmissibilityCondenser::CellAddress::ECLIPSE )
        {
            double trans =
                transCondenser.condensedTransmissibility( externalCell, { true, RigTransmissibilityCondenser::CellAddress::WELL, 1 } );

            if ( trans > transCondenser.transmissibilityThreshold() )
            {
                matrixToWellTransmissibilities.insert( std::make_pair( externalCell.m_globalCellIdx, trans ) );
            }
        }
    }
    return matrixToWellTransmissibilities;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData>
    RicExportFractureCompletionsImpl::generateCompdatValuesForFracture( const std::map<size_t, double>& matrixToWellTransmissibilites,
                                                                        const QString&                  wellNameForExport,
                                                                        const RimEclipseCase*           caseToApply,
                                                                        const RimFracture*              fracture,
                                                                        const RimFractureTemplate*      fracTemplate )
{
    std::vector<RigCompletionData> allCompletionsForOneFracture;
    for ( const auto& matrixToWellTransmissibility : matrixToWellTransmissibilites )
    {
        size_t            globalCellIndex = matrixToWellTransmissibility.first;
        double            trans           = matrixToWellTransmissibility.second;
        RigCompletionData compDat( wellNameForExport,
                                   RigCompletionDataGridCell( globalCellIndex, caseToApply->mainGrid() ),
                                   fracture->fractureMD() );

        double diameter = 2.0 * fracture->wellRadius();
        compDat.setFromFracture( trans, fracTemplate->skinFactor(), diameter );
        compDat.addMetadata( fracture->name(), QString::number( trans ) );
        compDat.setSourcePdmObject( fracture );
        allCompletionsForOneFracture.push_back( compDat );
    }

    if ( fracTemplate->isNonDarcyFlowEnabled() )
    {
        computeNonDarcyFlowParameters( fracture, allCompletionsForOneFracture );
    }

    return allCompletionsForOneFracture;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportFractureCompletionsImpl::computeNonDarcyFlowParameters( const RimFracture*              fracture,
                                                                      std::vector<RigCompletionData>& allCompletionsForOneFracture )
{
    double dFactorForFracture = fracture->nonDarcyProperties().dFactor;
    double khForFracture      = fracture->nonDarcyProperties().conductivity;

    double sumOfTransmissibilitiesInFracture = sumUpTransmissibilities( allCompletionsForOneFracture );

    for ( auto& c : allCompletionsForOneFracture )
    {
        // NOTE : What is supposed to happen when the transmissibility is close to zero?

        double dFactorForOneConnection = dFactorForFracture * sumOfTransmissibilitiesInFracture / c.transmissibility();
        c.setDFactor( dFactorForOneConnection );

        double khForOneConnection = khForFracture * c.transmissibility() / sumOfTransmissibilitiesInFracture;

        c.setKh( khForOneConnection );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicExportFractureCompletionsImpl::sumUpTransmissibilities( const std::vector<RigCompletionData>& allCompletionsForOneFracture )
{
    double transmissibility = 0.0;
    for ( const auto& c : allCompletionsForOneFracture )
    {
        transmissibility += c.transmissibility();
    }
    return transmissibility;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportFractureCompletionsImpl::calculateAndSetReportItemData( const std::vector<RigCompletionData>& allCompletionsForOneFracture,
                                                                      const RigEclipseToStimPlanCalculator& eclToFractureCalc,
                                                                      RicWellPathFractureReportItem&        reportItem )
{
    double areaWeightedMatrixPermeability = eclToFractureCalc.areaWeightedMatrixPermeability();
    reportItem.setAreaWeightedPermeability( areaWeightedMatrixPermeability );

    double totalAreaOpenForFlow     = eclToFractureCalc.totalEclipseAreaOpenForFlow();
    double areaWeightedConductivity = eclToFractureCalc.areaWeightedConductivity();

    if ( totalAreaOpenForFlow > 0.0 )
    {
        double halfLength = 0.0;
        double height     = eclToFractureCalc.longestYSectionOpenForFlow();
        if ( height > 0.0 )
        {
            double length = totalAreaOpenForFlow / height;
            halfLength    = length / 2.0;
        }

        reportItem.setHeightAndHalfLength( height, halfLength );
    }

    double aggregatedTransmissibility = sumUpTransmissibilities( allCompletionsForOneFracture );
    reportItem.setData( aggregatedTransmissibility, allCompletionsForOneFracture.size(), totalAreaOpenForFlow );

    reportItem.setWidthAndConductivity( eclToFractureCalc.areaWeightedWidth(), areaWeightedConductivity );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportFractureCompletionsImpl::outputIntermediateResultsText( QTextStream*                  outputStreamForIntermediateResultsText,
                                                                      const RimFracture*            fracture,
                                                                      RigTransmissibilityCondenser& transCondenser,
                                                                      const RigMainGrid*            mainGrid,
                                                                      const RigFractureGrid*        fractureGrid )
{
    ( *outputStreamForIntermediateResultsText )
        << "\n"
        << "\n"
        << "\n----------- All Transmissibilities " << fracture->name() << " -------------------- \n\n";

    ( *outputStreamForIntermediateResultsText ) << QString::fromStdString( transCondenser.neighborTransDebugOutput( mainGrid, fractureGrid ) );

    ( *outputStreamForIntermediateResultsText ) << "\n"
                                                << "\n"
                                                << "\n----------- Condensed Results " << fracture->name() << " -------------------- \n\n";

    ( *outputStreamForIntermediateResultsText )
        << QString::fromStdString( transCondenser.condensedTransDebugOutput( mainGrid, fractureGrid ) );

    ( *outputStreamForIntermediateResultsText ) << "\n";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportFractureCompletionsImpl::loadResultsByName( RigCaseCellResultsData* cellResultsData, const std::vector<QString>& resultNames )
{
    const std::vector<RiaDefines::ResultCatType> resultCategorySearchOrder = { RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                               RiaDefines::ResultCatType::INPUT_PROPERTY,
                                                                               RiaDefines::ResultCatType::GENERATED };

    bool foundDataForAllResults = true;

    if ( cellResultsData )
    {
        for ( const auto& resultName : resultNames )
        {
            if ( !cellResultsData->findAndLoadResultByName( resultName, resultCategorySearchOrder ) )
            {
                foundDataForAllResults = false;
            }
        }
    }

    return foundDataForAllResults;
}
