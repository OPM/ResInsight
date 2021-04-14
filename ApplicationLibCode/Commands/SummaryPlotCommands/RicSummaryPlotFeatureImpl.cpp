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

#include "RicSummaryPlotFeatureImpl.h"

#include "RiaApplication.h"
#include "RiaColorTables.h"
#include "RiaDefines.h"
#include "RiaEclipseFileNameTools.h"
#include "RiaImportEclipseCaseTools.h"
#include "RiaLogging.h"
#include "RiaPreferencesSummary.h"

#include "RicCreateSummaryCaseCollectionFeature.h"
#include "RicImportGeneralDataFeature.h"
#include "RicImportSummaryCasesFeature.h"
#include "WellLogCommands/RicWellLogPlotCurveFeatureImpl.h"

#include "RifSummaryReaderInterface.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"

#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseResultCase.h"
#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimGridTimeHistoryCurve.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

#include "RiuMainWindow.h"
#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"

#include <QFileInfo>
#include <QRegularExpression>
#include <QStringList>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCurve* RicSummaryPlotFeatureImpl::addDefaultCurveToPlot( RimSummaryPlot* plot, RimSummaryCase* summaryCase )
{
    if ( plot && summaryCase && summaryCase->summaryReader() )
    {
        RifEclipseSummaryAddress defaultAddressToUse;

        RiaPreferencesSummary* prefs = RiaPreferencesSummary::current();

        QString     curvesTextFilter = prefs->defaultSummaryCurvesTextFilter();
        QStringList curveFilters     = curvesTextFilter.split( ";", QString::SkipEmptyParts );

        if ( curveFilters.size() )
        {
            const std::set<RifEclipseSummaryAddress>& addrs = summaryCase->summaryReader()->allResultAddresses();

            for ( const auto& addr : addrs )
            {
                const QString& filter = curveFilters[0];
                {
                    if ( addr.isUiTextMatchingFilterText( filter ) )
                    {
                        defaultAddressToUse = addr;
                    }
                }
            }
        }

        RimSummaryCurve* newCurve = new RimSummaryCurve();

        // Use same counting as RicNewSummaryEnsembleCurveSetFeature::onActionTriggered
        cvf::Color3f curveColor =
            RiaColorTables::summaryCurveDefaultPaletteColors().cycledColor3f( plot->singleColorCurveCount() );
        newCurve->setColor( curveColor );

        plot->addCurveNoUpdate( newCurve );

        if ( summaryCase )
        {
            newCurve->setSummaryCaseY( summaryCase );
        }

        newCurve->setSummaryAddressYAndApplyInterpolation( defaultAddressToUse );

        return newCurve;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCurve*> RicSummaryPlotFeatureImpl::addDefaultCurvesToPlot( RimSummaryPlot* plot,
                                                                                 RimSummaryCase* summaryCase )
{
    RiaPreferencesSummary* prefs = RiaPreferencesSummary::current();

    QString     curvesTextFilter = prefs->defaultSummaryCurvesTextFilter();
    QStringList curveFilters     = curvesTextFilter.split( ";", QString::SkipEmptyParts );

    bool addHistoryCurve = false;

    return addCurvesFromAddressFiltersToPlot( curveFilters, plot, summaryCase, addHistoryCurve );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotFeatureImpl::ensureAtLeastOnePlot( RimSummaryPlotCollection* summaryPlotCollection,
                                                      RimSummaryCase*           summaryCase )
{
    if ( summaryPlotCollection && summaryCase )
    {
        if ( summaryPlotCollection->plots().empty() )
        {
            auto objectToSelect = createDefaultSummaryPlot( summaryCase );
            if ( objectToSelect )
            {
                RiuPlotMainWindowTools::setExpanded( objectToSelect );
                RiuPlotMainWindowTools::selectAsCurrentItem( objectToSelect );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObject* RicSummaryPlotFeatureImpl::createDefaultSummaryPlot( RimSummaryCase* summaryCase )
{
    RimSummaryPlotCollection* summaryPlotCollection = RimProject::current()->mainPlotCollection->summaryPlotCollection();

    RiaPreferencesSummary* prefs = RiaPreferencesSummary::current();

    caf::PdmObject* itemToSelect = nullptr;

    if ( summaryPlotCollection && summaryCase && !prefs->defaultSummaryCurvesTextFilter().isEmpty() )
    {
        auto plot = summaryPlotCollection->createSummaryPlotWithAutoTitle();

        std::vector<RimSummaryCurve*> curves = RicSummaryPlotFeatureImpl::addDefaultCurvesToPlot( plot, summaryCase );

        plot->applyDefaultCurveAppearances();
        plot->loadDataAndUpdate();

        summaryPlotCollection->updateConnectedEditors();

        if ( curves.size() )
        {
            itemToSelect = curves[0];
        }
        else
        {
            itemToSelect = plot;
        }
    }

    return itemToSelect;
}

RimSummaryCurve* createHistoryCurve( const RifEclipseSummaryAddress& addr, RimSummaryCase* summaryCasesToUse )
{
    RifEclipseSummaryAddress historyAddr = addr;
    historyAddr.setQuantityName( historyAddr.quantityName() + "H" );
    if ( summaryCasesToUse->summaryReader()->allResultAddresses().count( historyAddr ) )
    {
        RimSummaryCurve* historyCurve = new RimSummaryCurve();
        historyCurve->setSummaryCaseY( summaryCasesToUse );
        historyCurve->setSummaryAddressYAndApplyInterpolation( historyAddr );
        return historyCurve;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigGridCellResultAddress> RigGridCellResultAddress::createGridCellAddressesFromFilter( const QString& text )
{
    std::vector<RigGridCellResultAddress> addresses;
    QStringList                           addressParts = text.split( ":" );

    if ( addressParts.size() > 1 )
    {
        QString resultVarName = addressParts[0];
        size_t  gridIdx       = 0;
        if ( addressParts.size() > 2 )
        {
            gridIdx = addressParts[1].toULong();
        }

        QString     ijkText      = addressParts.back();
        QStringList ijkTextParts = ijkText.split( "," );
        if ( ijkTextParts.size() == 3 )
        {
            bool   isOk  = true;
            bool   allOk = true;
            size_t i     = ijkTextParts[0].toULong( &isOk );
            allOk &= isOk;
            size_t j = ijkTextParts[1].toULong( &isOk );
            allOk &= isOk;
            size_t k = ijkTextParts[2].toULong( &isOk );
            allOk &= isOk;

            if ( allOk )
            {
                addresses.emplace_back(
                    RigGridCellResultAddress( gridIdx, i - 1, j - 1, k - 1, RigEclipseResultAddress( resultVarName ) ) );
            }
        }
    }

    return addresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseCase*> openEclipseCasesForCellPlotting( QStringList gridFileNames )
{
    std::vector<RimEclipseCase*> openedCases;

    RiaApplication*           app            = RiaApplication::instance();
    RimProject*               project        = app->project();
    RimEclipseCaseCollection* analysisModels = project->activeOilField()->analysisModels();
    for ( const QString& fileName : gridFileNames )
    {
        QFileInfo gridFileInfo( fileName );

        if ( !gridFileInfo.exists() ) continue;

        QString caseName = gridFileInfo.completeBaseName();

        RimEclipseResultCase* rimResultReservoir = new RimEclipseResultCase();
        rimResultReservoir->setCaseInfo( caseName, fileName );

        analysisModels->cases.push_back( rimResultReservoir );

        if ( !rimResultReservoir->openReserviorCase() )
        {
            analysisModels->removeCaseFromAllGroups( rimResultReservoir );

            delete rimResultReservoir;

            continue;
        }
        else
        {
            openedCases.push_back( rimResultReservoir );
        }
    }

    analysisModels->updateConnectedEditors();

    return openedCases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotFeatureImpl::createSummaryPlotsFromArgumentLine( const QStringList& arguments )
{
    // Split arguments in options, vectors and filenames

    QStringList options;
    QStringList allCurveAddressFilters;
    QStringList summaryFileNames;
    QStringList gridFileNames;
    QString     ensembleColoringParameter;

    std::set<QString> validOptions = { "-help", "-h", "-nl", "-s", "-n", "-e", "-c", "-cl" };

    for ( int optionIdx = 0; optionIdx < arguments.size(); ++optionIdx )
    {
        if ( arguments[optionIdx].startsWith( "-" ) )
        {
            if ( arguments[optionIdx] == "-help" )
            {
                QString text = RicSummaryPlotFeatureImpl::summaryPlotCommandLineHelpText();
                RiaApplication::instance()->showFormattedTextInMessageBoxOrConsole( text );

                return;
            }

            if ( validOptions.count( arguments[optionIdx] ) )
            {
                options.push_back( arguments[optionIdx] );

                if ( arguments[optionIdx] == "-c" || arguments[optionIdx] == "-cl" )
                {
                    optionIdx++;
                    if ( optionIdx < arguments.size() ) ensembleColoringParameter = arguments[optionIdx];
                }
            }
            else
            {
                RiaLogging::error( "The summaryplot option: \"" + arguments[optionIdx] + "\" is unknown." );
            }
        }
        else
        {
            RiaEclipseFileNameTools nameTool( arguments[optionIdx] );
            QString                 smSpecFileName = nameTool.findRelatedSummarySpecFile();
            QString                 gridFileName   = nameTool.findRelatedGridFile();

            if ( smSpecFileName != "" || gridFileName != "" )
            {
                if ( smSpecFileName != "" ) summaryFileNames.push_back( smSpecFileName );
                if ( gridFileName != "" ) gridFileNames.push_back( gridFileName );
            }
            else
            {
                allCurveAddressFilters.push_back( arguments[optionIdx] );
            }
        }
    }

    summaryFileNames.removeDuplicates();
    gridFileNames.removeDuplicates();

    if ( allCurveAddressFilters.empty() )
    {
        RiaLogging::error( "Needs at least one vector to create a plot." );
    }

    bool hideLegend       = options.contains( "-nl" );
    bool addHistoryCurves = options.contains( "-h" );
    bool isNormalizedY    = options.contains( "-n" );
    bool isSinglePlot     = options.contains( "-s" );

    EnsembleColoringType ensembleColoringStyle = EnsembleColoringType::NONE;
    {
        int e_pos  = options.lastIndexOf( "-e" );
        int c_pos  = options.lastIndexOf( "-c" );
        int cl_pos = options.lastIndexOf( "-cl" );

        int lastEnsembleOptionPos = -1;
        if ( e_pos > lastEnsembleOptionPos )
        {
            lastEnsembleOptionPos = e_pos;
            ensembleColoringStyle = EnsembleColoringType::SINGLE_COLOR;
        }
        if ( c_pos > lastEnsembleOptionPos )
        {
            lastEnsembleOptionPos = c_pos;
            ensembleColoringStyle = EnsembleColoringType::PARAMETER;
        }
        if ( cl_pos > lastEnsembleOptionPos )
        {
            lastEnsembleOptionPos = cl_pos;
            ensembleColoringStyle = EnsembleColoringType::LOG_PARAMETER;
        }
    }

    bool isEnsembleMode = ensembleColoringStyle != EnsembleColoringType::NONE;

    std::vector<RimSummaryCase*> summaryCasesToUse;

    if ( summaryFileNames.size() )
    {
        RicImportSummaryCasesFeature::createSummaryCasesFromFiles( summaryFileNames, &summaryCasesToUse, isEnsembleMode );
        RicImportSummaryCasesFeature::addSummaryCases( summaryCasesToUse );

        RiaApplication::instance()->setLastUsedDialogDirectory( RiaDefines::defaultDirectoryLabel(
                                                                    RiaDefines::ImportFileType::ECLIPSE_SUMMARY_FILE ),
                                                                QFileInfo( summaryFileNames[0] ).absolutePath() );
    }

    if ( summaryCasesToUse.size() )
    {
        // Sort in summary and grid curve addresses
        QStringList gridResultAddressFilters;
        QStringList summaryAddressFilters;

        RimSummaryPlot* lastPlotCreated = nullptr;

        RimSummaryPlotCollection* sumPlotColl = RimProject::current()->mainPlotCollection()->summaryPlotCollection();

        splitAddressFiltersInGridAndSummary( summaryCasesToUse[0],
                                             allCurveAddressFilters,
                                             &summaryAddressFilters,
                                             &gridResultAddressFilters );

        if ( summaryAddressFilters.size() )
        {
            RimSummaryCaseCollection* ensemble = nullptr;

            if ( isEnsembleMode )
            {
                ensemble = RicCreateSummaryCaseCollectionFeature::groupSummaryCases( summaryCasesToUse, "Ensemble", true );
            }

            if ( isSinglePlot )
            {
                RimSummaryPlot* newPlot = createSummaryPlotFromAddresses( sumPlotColl,
                                                                          summaryCasesToUse,
                                                                          ensemble,
                                                                          summaryAddressFilters,
                                                                          addHistoryCurves,
                                                                          ensembleColoringStyle,
                                                                          ensembleColoringParameter );

                lastPlotCreated = newPlot;

                newPlot->setLegendsVisible( !hideLegend );
                newPlot->setNormalizationEnabled( isNormalizedY );
                newPlot->loadDataAndUpdate();
            }
            else // Multiplot, one for each separate summary address
            {
                std::vector<RimSummaryPlot*> summaryPlots =
                    createMultipleSummaryPlotsFromAddresses( sumPlotColl,
                                                             summaryCasesToUse,
                                                             ensemble,
                                                             summaryAddressFilters,
                                                             addHistoryCurves,
                                                             ensembleColoringStyle,
                                                             ensembleColoringParameter );

                lastPlotCreated = summaryPlots.back();

                for ( auto summaryPlot : summaryPlots )
                {
                    summaryPlot->setLegendsVisible( !hideLegend );
                    summaryPlot->setNormalizationEnabled( isNormalizedY );
                    summaryPlot->loadDataAndUpdate();
                }
            }
        }

        // Grid Cell Result vectors

        if ( gridResultAddressFilters.size() )
        {
            // Todo: Use identical grid case import if -e -c or -cl

            std::vector<RimEclipseCase*> gridCasesToPlotFrom = openEclipseCasesForCellPlotting( gridFileNames );

            if ( isSinglePlot )
            {
                std::vector<RimGridTimeHistoryCurve*> createdCurves;
                int                                   curveColorIndex = 0;
                for ( const QString& gridAddressFilter : gridResultAddressFilters )
                {
                    std::vector<RigGridCellResultAddress> cellResAddrs =
                        RigGridCellResultAddress::createGridCellAddressesFromFilter( gridAddressFilter );

                    for ( RigGridCellResultAddress cellResAddr : cellResAddrs )
                    {
                        for ( RimEclipseCase* eclCase : gridCasesToPlotFrom )
                        {
                            if ( !( eclCase->eclipseCaseData()->results( RiaDefines::PorosityModelType::MATRIX_MODEL ) &&
                                    eclCase->eclipseCaseData()
                                        ->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
                                        ->resultInfo( cellResAddr.eclipseResultAddress ) ) )
                            {
                                RiaLogging::warning( "Could not find a restart result property with name: \"" +
                                                     cellResAddr.eclipseResultAddress.resultName() + "\"" );
                                continue;
                            }

                            RimGridTimeHistoryCurve* newCurve = new RimGridTimeHistoryCurve();
                            newCurve->setFromEclipseCellAndResult( eclCase,
                                                                   cellResAddr.gridIndex,
                                                                   cellResAddr.i,
                                                                   cellResAddr.j,
                                                                   cellResAddr.k,
                                                                   cellResAddr.eclipseResultAddress );
                            newCurve->setLineThickness( 2 );
                            cvf::Color3f curveColor =
                                RicWellLogPlotCurveFeatureImpl::curveColorFromTable( curveColorIndex );
                            newCurve->setColor( curveColor );
                            if ( !isEnsembleMode ) ++curveColorIndex;

                            createdCurves.push_back( newCurve );
                        }
                        if ( isEnsembleMode ) ++curveColorIndex;
                    }
                }

                if ( createdCurves.size() )
                {
                    RimSummaryPlot* newPlot = sumPlotColl->createSummaryPlotWithAutoTitle();
                    for ( auto curve : createdCurves )
                    {
                        newPlot->addGridTimeHistoryCurve( curve );
                    }

                    newPlot->setLegendsVisible( !hideLegend );
                    newPlot->setNormalizationEnabled( isNormalizedY );
                    newPlot->loadDataAndUpdate();
                    lastPlotCreated = newPlot;
                }
            }
            else // Multiplot
            {
                int curveColorIndex = 0;

                for ( const QString& gridAddressFilter : gridResultAddressFilters )
                {
                    std::vector<RigGridCellResultAddress> cellResAddrs =
                        RigGridCellResultAddress::createGridCellAddressesFromFilter( gridAddressFilter );
                    for ( RigGridCellResultAddress cellResAddr : cellResAddrs )
                    {
                        std::vector<RimGridTimeHistoryCurve*> createdCurves;
                        for ( RimEclipseCase* eclCase : gridCasesToPlotFrom )
                        {
                            if ( !( eclCase->eclipseCaseData()->results( RiaDefines::PorosityModelType::MATRIX_MODEL ) &&
                                    eclCase->eclipseCaseData()
                                        ->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
                                        ->resultInfo( cellResAddr.eclipseResultAddress ) ) )
                            {
                                RiaLogging::warning( "Could not find a restart result property with name: \"" +
                                                     cellResAddr.eclipseResultAddress.resultName() + "\"" );
                                continue;
                            }
                            RimGridTimeHistoryCurve* newCurve = new RimGridTimeHistoryCurve();
                            newCurve->setFromEclipseCellAndResult( eclCase,
                                                                   cellResAddr.gridIndex,
                                                                   cellResAddr.i,
                                                                   cellResAddr.j,
                                                                   cellResAddr.k,
                                                                   cellResAddr.eclipseResultAddress );
                            newCurve->setLineThickness( 2 );
                            cvf::Color3f curveColor =
                                RicWellLogPlotCurveFeatureImpl::curveColorFromTable( curveColorIndex );
                            newCurve->setColor( curveColor );
                            if ( !isEnsembleMode ) ++curveColorIndex;
                            createdCurves.push_back( newCurve );
                        }

                        if ( isEnsembleMode ) ++curveColorIndex;

                        if ( createdCurves.size() )
                        {
                            RimSummaryPlot* newPlot = sumPlotColl->createSummaryPlotWithAutoTitle();
                            for ( auto newCurve : createdCurves )
                            {
                                newPlot->addGridTimeHistoryCurve( newCurve );
                            }
                            newPlot->setLegendsVisible( !hideLegend );
                            newPlot->setNormalizationEnabled( isNormalizedY );
                            newPlot->loadDataAndUpdate();
                            lastPlotCreated = newPlot;
                        }
                    }
                }
            }
        }

        if ( lastPlotCreated )
        {
            sumPlotColl->updateConnectedEditors();

            RiuPlotMainWindow* mpw = RiaGuiApplication::instance()->mainPlotWindow();
            // Needed to avoid unnecessary activation of sub windows (plots)
            // which results in population of property editor, and missing deleteLater because we are outside any event
            // loop when switching object. Results in stray widgets.
            mpw->setBlockViewSelectionOnSubWindowActivated( true );
            RiuPlotMainWindowTools::showPlotMainWindow();
            mpw->setBlockViewSelectionOnSubWindowActivated( false );
            RiuPlotMainWindowTools::setExpanded( lastPlotCreated );
            RiuPlotMainWindowTools::selectAsCurrentItem( lastPlotCreated );

            RiuMainWindow::instance()->close();
        }
    }
    else
    {
        RiaLogging::error( "Needs at least one summary case to create a plot." );
    }
}

RimSummaryPlot*
    RicSummaryPlotFeatureImpl::createSummaryPlotFromAddresses( RimSummaryPlotCollection*           sumPlotColl,
                                                               const std::vector<RimSummaryCase*>& summaryCasesToUse,
                                                               RimSummaryCaseCollection*           ensemble,
                                                               QStringList          summaryAddressFilters,
                                                               bool                 addHistoryCurves,
                                                               EnsembleColoringType ensembleColoringStyle,
                                                               QString              ensembleColoringParameter )
{
    RimSummaryPlot* newPlot = sumPlotColl->createSummaryPlotWithAutoTitle();

    if ( ensemble )
    {
        std::set<RifEclipseSummaryAddress> filteredAdressesFromCases =
            applySummaryAddressFiltersToCases( summaryCasesToUse, summaryAddressFilters );

        for ( const auto& addr : filteredAdressesFromCases )
        {
            RimEnsembleCurveSet* curveSet = new RimEnsembleCurveSet();
            curveSet->setSummaryCaseCollection( ensemble );
            curveSet->setSummaryAddress( addr );

            if ( ensembleColoringStyle == EnsembleColoringType::PARAMETER ||
                 ensembleColoringStyle == EnsembleColoringType::LOG_PARAMETER )
            {
                curveSet->setColorMode( RimEnsembleCurveSet::ColorMode::BY_ENSEMBLE_PARAM );
                curveSet->setEnsembleParameter( ensembleColoringParameter );

                if ( ensembleColoringStyle == EnsembleColoringType::LOG_PARAMETER )
                {
                    curveSet->legendConfig()->setMappingMode( RimRegularLegendConfig::MappingType::LOG10_CONTINUOUS );
                }
            }

            newPlot->ensembleCurveSetCollection()->addCurveSet( curveSet );

            if ( addHistoryCurves )
            {
                RimSummaryCurve* historyCurve = createHistoryCurve( addr, summaryCasesToUse[0] );

                if ( historyCurve ) newPlot->addCurveNoUpdate( historyCurve );
            }
        }
    }
    else
    {
        for ( RimSummaryCase* sumCase : summaryCasesToUse )
        {
            RicSummaryPlotFeatureImpl::addCurvesFromAddressFiltersToPlot( summaryAddressFilters,
                                                                          newPlot,
                                                                          sumCase,
                                                                          addHistoryCurves );
            addHistoryCurves = false;
        }
    }

    newPlot->applyDefaultCurveAppearances();

    return newPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryPlot*> RicSummaryPlotFeatureImpl::createMultipleSummaryPlotsFromAddresses(
    RimSummaryPlotCollection*           sumPlotColl,
    const std::vector<RimSummaryCase*>& summaryCasesToUse,
    RimSummaryCaseCollection*           ensemble,
    QStringList                         summaryAddressFilters,
    bool                                addHistoryCurves,
    EnsembleColoringType                ensembleColoringStyle /*= EnsembleColoringType::NONE*/,
    QString                             ensembleColoringParameter /*= "" */ )
{
    std::vector<RimSummaryPlot*> newSummaryPlots;

    std::set<RifEclipseSummaryAddress> filteredAdressesFromCases =
        applySummaryAddressFiltersToCases( summaryCasesToUse, summaryAddressFilters );

    for ( const auto& addr : filteredAdressesFromCases )
    {
        std::vector<RimSummaryCurve*>     createdCurves;
        std::vector<RimEnsembleCurveSet*> createdEnsembleCurveSets;
        if ( ensemble )
        {
            RimEnsembleCurveSet* curveSet = new RimEnsembleCurveSet();

            curveSet->setSummaryCaseCollection( ensemble );
            curveSet->setSummaryAddress( addr );
            if ( ensembleColoringStyle == EnsembleColoringType::PARAMETER ||
                 ensembleColoringStyle == EnsembleColoringType::LOG_PARAMETER )
            {
                curveSet->setColorMode( RimEnsembleCurveSet::ColorMode::BY_ENSEMBLE_PARAM );
                curveSet->setEnsembleParameter( ensembleColoringParameter );
                if ( ensembleColoringStyle == EnsembleColoringType::LOG_PARAMETER )
                {
                    curveSet->legendConfig()->setMappingMode( RimRegularLegendConfig::MappingType::LOG10_CONTINUOUS );
                }
            }
            createdEnsembleCurveSets.push_back( curveSet );
        }
        else
        {
            for ( RimSummaryCase* sumCase : summaryCasesToUse )
            {
                const std::set<RifEclipseSummaryAddress>& allAddrsInCase = sumCase->summaryReader()->allResultAddresses();
                if ( allAddrsInCase.count( addr ) )
                {
                    RimSummaryCurve* newCurve = new RimSummaryCurve();
                    newCurve->setSummaryCaseY( sumCase );
                    newCurve->setSummaryAddressYAndApplyInterpolation( addr );
                    createdCurves.push_back( newCurve );
                }
            }
        }

        if ( addHistoryCurves )
        {
            RimSummaryCurve* historyCurve = createHistoryCurve( addr, summaryCasesToUse[0] );
            if ( historyCurve ) createdCurves.push_back( historyCurve );
        }

        if ( createdCurves.size() || createdEnsembleCurveSets.size() )
        {
            RimSummaryPlot* newPlot = sumPlotColl->createSummaryPlotWithAutoTitle();

            for ( auto curve : createdCurves )
            {
                newPlot->addCurveNoUpdate( curve );
            }

            for ( auto curveSet : createdEnsembleCurveSets )
            {
                newPlot->ensembleCurveSetCollection()->addCurveSet( curveSet );
            }

            newPlot->applyDefaultCurveAppearances();
            newSummaryPlots.push_back( newPlot );
        }
    }
    return newSummaryPlots;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotFeatureImpl::splitAddressFiltersInGridAndSummary( RimSummaryCase*    summaryBaseCases,
                                                                     const QStringList& allCurveAddressFilters,
                                                                     QStringList*       summaryAddressFilters,
                                                                     QStringList*       gridResultAddressFilters )
{
    if ( summaryBaseCases )
    {
        const std::set<RifEclipseSummaryAddress>& addrs = summaryBaseCases->summaryReader()->allResultAddresses();
        std::vector<bool>                         usedFilters;
        std::set<RifEclipseSummaryAddress>        setToInsertFilteredAddressesIn;
        filteredSummaryAdressesFromCase( allCurveAddressFilters, addrs, &setToInsertFilteredAddressesIn, &usedFilters );

        QRegularExpression gridAddressPattern( "^[A-Z]+:[0-9]+,[0-9]+,[0-9]+$" );

        for ( int filterIdx = 0; filterIdx < allCurveAddressFilters.size(); ++filterIdx )
        {
            const QString& address = allCurveAddressFilters[filterIdx];
            if ( usedFilters[filterIdx] )
            {
                summaryAddressFilters->push_back( address );
            }
            else
            {
                if ( gridAddressPattern.match( address ).hasMatch() )
                {
                    gridResultAddressFilters->push_back( address );
                }
                else
                {
                    RiaLogging::warning( "No summary or restart vectors matched \"" + address + "\"" );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress>
    RicSummaryPlotFeatureImpl::applySummaryAddressFiltersToCases( const std::vector<RimSummaryCase*>& summaryCasesToUse,
                                                                  const QStringList& summaryAddressFilters )
{
    std::set<RifEclipseSummaryAddress> filteredAdressesFromCases;
    for ( RimSummaryCase* sumCase : summaryCasesToUse )
    {
        const std::set<RifEclipseSummaryAddress>& addrs = sumCase->summaryReader()->allResultAddresses();
        std::vector<bool>                         usedFilters;

        filteredSummaryAdressesFromCase( summaryAddressFilters, addrs, &filteredAdressesFromCases, &usedFilters );

        for ( size_t cfIdx = 0; cfIdx < usedFilters.size(); ++cfIdx )
        {
            if ( !usedFilters[cfIdx] )
            {
                RiaLogging::warning( "Vector filter \"" + summaryAddressFilters[static_cast<int>( cfIdx )] +
                                     "\" did not match anything in case: \"" + sumCase->nativeCaseName() + "\"" );
            }
        }
    }
    return filteredAdressesFromCases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCurve*> RicSummaryPlotFeatureImpl::addCurvesFromAddressFiltersToPlot( const QStringList& curveFilters,
                                                                                            RimSummaryPlot* plot,
                                                                                            RimSummaryCase* summaryCase,
                                                                                            bool addHistoryCurves )
{
    std::vector<RimSummaryCurve*> createdCurves;

    if ( !plot ) return createdCurves;
    if ( !summaryCase || !summaryCase->summaryReader() ) return createdCurves;

    std::set<RifEclipseSummaryAddress> curveAddressesToUse;

    const std::set<RifEclipseSummaryAddress>& addrs = summaryCase->summaryReader()->allResultAddresses();
    std::vector<bool>                         usedFilters;

    filteredSummaryAdressesFromCase( curveFilters, addrs, &curveAddressesToUse, &usedFilters );

    for ( size_t cfIdx = 0; cfIdx < usedFilters.size(); ++cfIdx )
    {
        if ( !usedFilters[cfIdx] )
        {
            RiaLogging::warning( "Vector filter \"" + curveFilters[static_cast<int>( cfIdx )] +
                                 "\" did not match anything in case: \"" + summaryCase->nativeCaseName() + "\"" );
        }
    }

    if ( addHistoryCurves )
    {
        std::vector<RifEclipseSummaryAddress> historyAddressesToUse;
        for ( RifEclipseSummaryAddress historyAddr : curveAddressesToUse )
        {
            historyAddr.setQuantityName( historyAddr.quantityName() + "H" );
            if ( addrs.count( historyAddr ) )
            {
                historyAddressesToUse.push_back( historyAddr );
            }
        }
        curveAddressesToUse.insert( historyAddressesToUse.begin(), historyAddressesToUse.end() );
    }

    if ( !curveFilters.isEmpty() && curveAddressesToUse.empty() && !addrs.empty() )
    {
        // The curve filter returns no match, use first available address

        curveAddressesToUse.insert( *addrs.begin() );
    }

    for ( const auto& addr : curveAddressesToUse )
    {
        RimSummaryCurve* newCurve = new RimSummaryCurve();
        plot->addCurveNoUpdate( newCurve );
        if ( summaryCase )
        {
            newCurve->setSummaryCaseY( summaryCase );
        }
        newCurve->setSummaryAddressYAndApplyInterpolation( addr );
        createdCurves.push_back( newCurve );
    }

    return createdCurves;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotFeatureImpl::filteredSummaryAdressesFromCase(
    const QStringList&                        curveFilters,
    const std::set<RifEclipseSummaryAddress>& allAddressesInCase,
    std::set<RifEclipseSummaryAddress>*       setToInsertFilteredAddressesIn,
    std::vector<bool>*                        usedFilters )
{
    int curveFilterCount = curveFilters.size();

    usedFilters->clear();
    usedFilters->resize( curveFilterCount, false );

    for ( const auto& addr : allAddressesInCase )
    {
        for ( int cfIdx = 0; cfIdx < curveFilterCount; ++cfIdx )
        {
            if ( addr.isUiTextMatchingFilterText( curveFilters[cfIdx] ) )
            {
                setToInsertFilteredAddressesIn->insert( addr );
                ( *usedFilters )[cfIdx] = true;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicSummaryPlotFeatureImpl::summaryPlotCommandLineHelpText()
{
    // clang-format off
    QString txt =
    "The --summaryplot option has the following syntax:\n"
    "\n"
    "[<plotOptions>] <eclipsesummaryvectorfilters> <eclipsedatafiles>\n"
    "\n"
    "It creates one summary plot for each of the the summary vectors matched by the "
    "<eclipsesummaryvectorfilters> using all the <eclipsedatafiles> in each plot.\n"
    "The <eclipsesummaryvectorfilters> has the syntax <vectorname>[:<item>[:<subitem>[:i,j,k]]] and can be repeated.\n"
    "Wildcards can also be used, eg. \"WOPT:*\" to select the total oil production from all the wells.\n"
    "3D Grid properties from restart files can also be requested in the form <propertyname>:i,j,k.\n"
    "The <eclipsedatafiles> can be written with or without extension.\n"
    "As long as only summary vectors are requested, only the corresponding SMSPEC file will be opened for each case.\n"
    "If a grid property is requested, however (eg. SOIL:20,21,1) the corresponding EGRID and restart data will be loaded as well.\n"
    "\n"
    "The summary plot options are: \n"
    "  -help\t Show this help text and ignore the rest of the options.\n"
    "  -h\t Include history vectors. Will be read from the summary file if the vectors exist.\n"
    "    \t Only history vectors from the first summary case in the project will be included.\n"
    "  -nl\t Omit legend in plot.\n"
    "  -s\t Create only one plot including all the defined vectors and cases.\n"
    "  -n\t Scale all curves into the range 0.0-1.0. Useful when using -s.\n"
    "  -e\t Import all the cases as an ensemble, and create ensemble curves sets instead of single curves.\n"
    "  -c  <parametername>\t Same as -e, but colors the curves by the ensemble parameter <parametername> . \n"
    "  -cl <parametername>\t Same as -c, but uses logarithmic legend.\n";
    // clang-format on

    return txt;
}
