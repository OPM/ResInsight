/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RimEclipseCase.h"

#include "RiaApplication.h"
#include "RiaColorTables.h"
#include "RiaFieldHandleTools.h"
#include "RiaPreferences.h"
#include "RiaQDateTimeTools.h"

#include "CompletionExportCommands/RicWellPathExportCompletionDataFeatureImpl.h"

#include "RicfCommandObject.h"
#include "RifReaderSettings.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigSimWellData.h"
#include "RigVirtualPerforationTransmissibilities.h"

#include "Rim2dIntersectionViewCollection.h"
#include "RimCaseCollection.h"
#include "RimCellEdgeColors.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseContourMapView.h"
#include "RimEclipseContourMapViewCollection.h"
#include "RimEclipseInputPropertyCollection.h"
#include "RimEclipsePropertyFilter.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseStatisticsCase.h"
#include "RimEclipseView.h"
#include "RimFaultInViewCollection.h"
#include "RimFormationNames.h"
#include "RimGridCollection.h"
#include "RimIntersectionCollection.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimPerforationCollection.h"
#include "RimProject.h"
#include "RimReloadCaseTools.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimStimPlanColors.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "cafPdmDocument.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafProgressInfo.h"

#include "cafUtils.h"
#include <QFileInfo>

CAF_PDM_XML_ABSTRACT_SOURCE_INIT( RimEclipseCase, "RimReservoir" );

//------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase::RimEclipseCase()
{
    RICF_InitObjectWithScriptNameAndComment( "EclipseCase",
                                             ":/Case48x48.png",
                                             "",
                                             "",
                                             "Reservoir",
                                             "Abtract base class for Eclipse Cases" );

    CAF_PDM_InitFieldNoDefault( &reservoirViews, "ReservoirViews", "", "", "", "" );
    reservoirViews.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_matrixModelResults, "MatrixModelResults", "", "", "", "" );
    m_matrixModelResults.uiCapability()->setUiHidden( true );
    CAF_PDM_InitFieldNoDefault( &m_fractureModelResults, "FractureModelResults", "", "", "", "" );
    m_fractureModelResults.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_flipXAxis, "FlipXAxis", false, "Flip X Axis", "", "", "" );
    CAF_PDM_InitField( &m_flipYAxis, "FlipYAxis", false, "Flip Y Axis", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_filesContainingFaults, "CachedFileNamesContainingFaults", "", "", "", "" );
    m_filesContainingFaults.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_contourMapCollection, "ContourMaps", "2d Contour Maps", "", "", "" );
    m_contourMapCollection = new RimEclipseContourMapViewCollection;
    m_contourMapCollection.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_inputPropertyCollection, "InputPropertyCollection", "", "", "", "" );
    m_inputPropertyCollection = new RimEclipseInputPropertyCollection;
    m_inputPropertyCollection->parentField()->uiCapability()->setUiHidden( true );

    // Obsolete fields
    CAF_PDM_InitFieldNoDefault( &m_filesContainingFaults_OBSOLETE, "FilesContainingFaults", "", "", "", "" );
    RiaFieldhandleTools::disableWriteAndSetFieldHidden( &m_filesContainingFaults_OBSOLETE );

    CAF_PDM_InitField( &m_caseName_OBSOLETE, "CaseName", QString(), "Obsolete", "", "", "" );
    RiaFieldhandleTools::disableWriteAndSetFieldHidden( &m_caseName_OBSOLETE );

    // Init

    m_matrixModelResults = new RimReservoirCellResultsStorage;
    m_matrixModelResults.uiCapability()->setUiHidden( true );
    m_matrixModelResults.uiCapability()->setUiTreeChildrenHidden( true );

    m_fractureModelResults = new RimReservoirCellResultsStorage;
    m_fractureModelResults.uiCapability()->setUiHidden( true );
    m_fractureModelResults.uiCapability()->setUiTreeChildrenHidden( true );

    this->setReservoirData( nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase::~RimEclipseCase()
{
    reservoirViews.deleteAllChildObjects();

    delete m_matrixModelResults();
    delete m_fractureModelResults();
    delete m_inputPropertyCollection;

    RimProject* project = RiaApplication::instance()->project();
    if ( project )
    {
        if ( project->mainPlotCollection() )
        {
            RimWellLogPlotCollection* plotCollection = project->mainPlotCollection()->wellLogPlotCollection();
            if ( plotCollection )
            {
                plotCollection->removeExtractors( this->eclipseCaseData() );
            }
        }
    }

    if ( this->eclipseCaseData() )
    {
        // At this point, we assume that memory should be released
        CVF_ASSERT( this->eclipseCaseData()->refCount() == 1 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseCaseData* RimEclipseCase::eclipseCaseData()
{
    return m_rigEclipseCase.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigEclipseCaseData* RimEclipseCase::eclipseCaseData() const
{
    return m_rigEclipseCase.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::ensureDeckIsParsedForEquilData()
{
    if ( m_rigEclipseCase.notNull() )
    {
        QString includeFileAbsolutePathPrefix;
        {
            RiaPreferences* prefs = RiaApplication::instance()->preferences();
            if ( prefs->readerSettings() )
            {
                includeFileAbsolutePathPrefix = prefs->readerSettings()->includeFileAbsolutePathPrefix();
            }
        }

        QString dataDeckFile;
        {
            QFileInfo fi( gridFileName() );

            dataDeckFile = caf::Utils::constructFullFileName( fi.absolutePath(), fi.baseName(), ".DATA" );
        }

        m_rigEclipseCase->ensureDeckIsParsedForEquilData( dataDeckFile, includeFileAbsolutePathPrefix );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimEclipseCase::defaultWellColor( const QString& wellName )
{
    if ( m_wellToColorMap.empty() )
    {
        const caf::ColorTable& colorTable             = RiaColorTables::wellsPaletteColors();
        cvf::Color3ubArray     wellColors             = colorTable.color3ubArray();
        cvf::Color3ubArray     interpolatedWellColors = wellColors;

        const cvf::Collection<RigSimWellData>& simWellData = this->eclipseCaseData()->wellResults();
        if ( simWellData.size() > 1 )
        {
            interpolatedWellColors = caf::ColorTable::interpolateColorArray( wellColors, simWellData.size() );
        }

        for ( size_t wIdx = 0; wIdx < simWellData.size(); ++wIdx )
        {
            m_wellToColorMap[simWellData[wIdx]->m_wellName] = cvf::Color3f::BLACK;
        }

        size_t wIdx = 0;
        for ( auto& wNameColorPair : m_wellToColorMap )
        {
            wNameColorPair.second = cvf::Color3f( interpolatedWellColors[wIdx] );

            ++wIdx;
        }
    }

    auto nmColor = m_wellToColorMap.find( wellName );
    if ( nmColor != m_wellToColorMap.end() )
    {
        return nmColor->second;
    }
    else
    {
        return cvf::Color3f::LIGHT_GRAY;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigMainGrid* RimEclipseCase::mainGrid() const
{
    if ( eclipseCaseData() )
    {
        return eclipseCaseData()->mainGrid();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::initAfterRead()
{
    RimCase::initAfterRead();

    size_t j;
    for ( j = 0; j < reservoirViews().size(); j++ )
    {
        RimEclipseView* riv = reservoirViews()[j];
        CVF_ASSERT( riv );

        riv->setEclipseCase( this );
    }
    for ( RimEclipseContourMapView* contourMap : m_contourMapCollection->views() )
    {
        contourMap->setEclipseCase( this );
    }

    if ( caseUserDescription().isEmpty() && !m_caseName_OBSOLETE().isEmpty() )
    {
        caseUserDescription = m_caseName_OBSOLETE;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseView* RimEclipseCase::createAndAddReservoirView()
{
    RimEclipseView* rimEclipseView = new RimEclipseView();

    rimEclipseView->setEclipseCase( this );

    // Set default values
    {
        rimEclipseView->cellResult()->setResultType( RiaDefines::DYNAMIC_NATIVE );

        auto prefs = RiaApplication::instance()->preferences();
        if ( prefs->loadAndShowSoil )
        {
            rimEclipseView->cellResult()->setResultVariable( "SOIL" );
        }

        rimEclipseView->faultCollection()->showFaultCollection = prefs->enableFaultsByDefault();

        rimEclipseView->hasUserRequestedAnimation = true;

        rimEclipseView->cellEdgeResult()->setResultVariable( "MULT" );
        rimEclipseView->cellEdgeResult()->setActive( false );
        rimEclipseView->fractureColors()->setDefaultResultName();
    }

    caf::PdmDocument::updateUiIconStateRecursively( rimEclipseView );

    reservoirViews().push_back( rimEclipseView );

    return rimEclipseView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseView* RimEclipseCase::createCopyAndAddView( const RimEclipseView* sourceView )
{
    CVF_ASSERT( sourceView );

    RimEclipseView* rimEclipseView = dynamic_cast<RimEclipseView*>(
        sourceView->xmlCapability()->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );
    CVF_ASSERT( rimEclipseView );
    rimEclipseView->setEclipseCase( this );

    caf::PdmDocument::updateUiIconStateRecursively( rimEclipseView );

    reservoirViews().push_back( rimEclipseView );

    // Resolve references after reservoir view has been inserted into Rim structures
    rimEclipseView->resolveReferencesRecursively();
    rimEclipseView->initAfterReadRecursively();

    return rimEclipseView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigVirtualPerforationTransmissibilities* RimEclipseCase::computeAndGetVirtualPerforationTransmissibilities()
{
    RigEclipseCaseData* rigEclipseCase = eclipseCaseData();
    if ( !rigEclipseCase ) return nullptr;

    if ( rigEclipseCase->virtualPerforationTransmissibilities() == nullptr )
    {
        cvf::ref<RigVirtualPerforationTransmissibilities> perfTrans = new RigVirtualPerforationTransmissibilities;

        std::vector<RimWellPath*> visibleWellPaths;
        bool                      anyPerforationsPresent = false;
        {
            RimProject*               proj      = RiaApplication::instance()->project();
            std::vector<RimWellPath*> wellPaths = proj->allWellPaths();
            for ( auto w : wellPaths )
            {
                if ( w->showWellPath() )
                {
                    visibleWellPaths.push_back( w );

                    if ( !w->perforationIntervalCollection()->perforations().empty() )
                    {
                        anyPerforationsPresent = true;
                    }
                }
            }
        }

        for ( auto w : visibleWellPaths )
        {
            std::vector<RigCompletionData> staticCompletionData =
                RicWellPathExportCompletionDataFeatureImpl::computeStaticCompletionsForWellPath( w, this );

            if ( anyPerforationsPresent )
            {
                std::vector<std::vector<RigCompletionData>> allCompletionData;

                for ( size_t i = 0; i < timeStepDates().size(); i++ )
                {
                    std::vector<RigCompletionData> dynamicCompletionDataOneTimeStep =
                        RicWellPathExportCompletionDataFeatureImpl::computeDynamicCompletionsForWellPath( w, this, i );

                    std::copy( staticCompletionData.begin(),
                               staticCompletionData.end(),
                               std::back_inserter( dynamicCompletionDataOneTimeStep ) );

                    allCompletionData.push_back( dynamicCompletionDataOneTimeStep );
                }

                perfTrans->setCompletionDataForWellPath( w, allCompletionData );
            }
            else
            {
                std::vector<std::vector<RigCompletionData>> allCompletionData;
                allCompletionData.push_back( staticCompletionData );

                perfTrans->setCompletionDataForWellPath( w, allCompletionData );
            }
        }

        for ( const auto& wellRes : rigEclipseCase->wellResults() )
        {
            std::vector<std::vector<RigCompletionData>> completionsPerTimeStep;
            for ( size_t i = 0; i < timeStepDates().size(); i++ )
            {
                std::vector<RigCompletionData> completionData;

                if ( wellRes->hasWellResult( i ) )
                {
                    for ( const auto& wellResultBranch : wellRes->wellResultFrame( i ).m_wellResultBranches )
                    {
                        for ( const auto& r : wellResultBranch.m_branchResultPoints )
                        {
                            if ( r.isCell() )
                            {
                                RigCompletionData compData( wellRes->m_wellName,
                                                            RigCompletionDataGridCell( r.m_gridCellIndex,
                                                                                       rigEclipseCase->mainGrid() ),
                                                            0 );
                                compData.setTransmissibility( r.connectionFactor() );

                                completionData.push_back( compData );
                            }
                        }
                    }
                }

                completionsPerTimeStep.push_back( completionData );

                perfTrans->setCompletionDataForSimWell( wellRes.p(), completionsPerTimeStep );
            }
        }

        rigEclipseCase->setVirtualPerforationTransmissibilities( perfTrans.p() );
    }

    return rigEclipseCase->virtualPerforationTransmissibilities();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                       const QVariant&            oldValue,
                                       const QVariant&            newValue )
{
    if ( changedField == &m_releaseResultMemory )
    {
        RimReloadCaseTools::reloadAllEclipseGridData( this );

        m_releaseResultMemory = oldValue.toBool();
    }
    else if ( changedField == &m_flipXAxis || changedField == &m_flipYAxis )
    {
        RigEclipseCaseData* rigEclipseCase = eclipseCaseData();
        if ( rigEclipseCase )
        {
            rigEclipseCase->mainGrid()->setFlipAxis( m_flipXAxis, m_flipYAxis );

            computeCachedData();

            for ( size_t i = 0; i < reservoirViews().size(); i++ )
            {
                RimEclipseView* reservoirView = reservoirViews()[i];

                reservoirView->scheduleReservoirGridGeometryRegen();
                reservoirView->scheduleSimWellGeometryRegen();
                reservoirView->createDisplayModelAndRedraw();
            }
        }
    }
    else if ( changedField == &m_activeFormationNames )
    {
        updateFormationNamesData();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::updateFormationNamesData()
{
    RigEclipseCaseData* rigEclipseCase = eclipseCaseData();
    if ( rigEclipseCase )
    {
        if ( activeFormationNames() )
        {
            rigEclipseCase->setActiveFormationNames( activeFormationNames()->formationNamesData() );
        }
        else
        {
            rigEclipseCase->setActiveFormationNames( nullptr );
        }

        // Update plots based on formations
        {
            RimProject* project = RiaApplication::instance()->project();
            if ( project )
            {
                if ( project->mainPlotCollection() )
                {
                    project->mainPlotCollection->updatePlotsWithFormations();
                }
            }
        }

        std::vector<Rim3dView*> views = this->views();
        for ( Rim3dView* view : views )
        {
            RimEclipseView* eclView = dynamic_cast<RimEclipseView*>( view );

            if ( eclView && eclView->isUsingFormationNames() )
            {
                if ( !activeFormationNames() )
                {
                    if ( eclView->cellResult()->resultType() == RiaDefines::FORMATION_NAMES )
                    {
                        eclView->cellResult()->setResultVariable( RiaDefines::undefinedResultName() );
                        eclView->cellResult()->updateConnectedEditors();
                    }

                    RimEclipsePropertyFilterCollection* eclFilColl = eclView->eclipsePropertyFilterCollection();
                    for ( RimEclipsePropertyFilter* propFilter : eclFilColl->propertyFilters )
                    {
                        if ( propFilter->resultDefinition()->resultType() == RiaDefines::FORMATION_NAMES )
                        {
                            propFilter->resultDefinition()->setResultVariable( RiaDefines::undefinedResultName() );
                        }
                    }
                }

                RimEclipsePropertyFilterCollection* eclFilColl = eclView->eclipsePropertyFilterCollection();
                for ( RimEclipsePropertyFilter* propFilter : eclFilColl->propertyFilters )
                {
                    if ( propFilter->resultDefinition()->resultType() == RiaDefines::FORMATION_NAMES )
                    {
                        propFilter->setToDefaultValues();
                        propFilter->updateConnectedEditors();
                    }
                }

                view->scheduleGeometryRegen( PROPERTY_FILTERED );
                view->scheduleCreateDisplayModelAndRedraw();
                eclView->intersectionCollection()->scheduleCreateDisplayModelAndRedraw2dIntersectionViews();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    std::vector<PdmObjectHandle*> children;
    reservoirViews.childObjects( &children );

    for ( auto child : children )
        uiTreeOrdering.add( child );

    if ( !m_2dIntersectionViewCollection->views().empty() )
    {
        uiTreeOrdering.add( &m_2dIntersectionViewCollection );
    }

    if ( !m_contourMapCollection->views().empty() )
    {
        uiTreeOrdering.add( &m_contourMapCollection );
    }

    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::computeCachedData()
{
    RigEclipseCaseData* rigEclipseCase = eclipseCaseData();
    if ( rigEclipseCase )
    {
        caf::ProgressInfo pInf( 30, "" );

        {
            auto task = pInf.task( "", 1 );
            rigEclipseCase->computeActiveCellBoundingBoxes();
        }

        {
            auto task = pInf.task( "Calculating Cell Search Tree", 10 );
            rigEclipseCase->mainGrid()->computeCachedData();
        }

        {
            auto task = pInf.task( "Calculating faults", 17 );
            rigEclipseCase->mainGrid()->calculateFaults( rigEclipseCase->activeCellInfo( RiaDefines::MATRIX_MODEL ) );
        }

        {
            auto task = pInf.task( "Calculating Formation Names Result", 2 );
            if ( activeFormationNames() )
            {
                rigEclipseCase->setActiveFormationNames( activeFormationNames()->formationNamesData() );
            }
            else
            {
                rigEclipseCase->setActiveFormationNames( nullptr );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCaseCollection* RimEclipseCase::parentCaseCollection()
{
    return dynamic_cast<RimCaseCollection*>( this->parentField()->ownerObject() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseContourMapViewCollection* RimEclipseCase::contourMapCollection()
{
    return m_contourMapCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseInputPropertyCollection* RimEclipseCase::inputPropertyCollection()
{
    return m_inputPropertyCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::setReservoirData( RigEclipseCaseData* eclipseCase )
{
    m_rigEclipseCase = eclipseCase;
    if ( this->eclipseCaseData() )
    {
        m_fractureModelResults()->setCellResults( eclipseCaseData()->results( RiaDefines::FRACTURE_MODEL ) );
        m_matrixModelResults()->setCellResults( eclipseCaseData()->results( RiaDefines::MATRIX_MODEL ) );
    }
    else
    {
        m_fractureModelResults()->setCellResults( nullptr );
        m_matrixModelResults()->setCellResults( nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::createTimeStepFormatString()
{
    std::vector<QDateTime> timeStepDates = this->timeStepDates();

    m_timeStepFormatString = RiaQDateTimeTools::createTimeFormatStringFromDates( timeStepDates );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimEclipseCase::reservoirBoundingBox()
{
    return activeCellsBoundingBox();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimEclipseCase::activeCellsBoundingBox() const
{
    if ( m_rigEclipseCase.notNull() && m_rigEclipseCase->activeCellInfo( RiaDefines::MATRIX_MODEL ) )
    {
        return m_rigEclipseCase->activeCellInfo( RiaDefines::MATRIX_MODEL )->geometryBoundingBox();
    }
    else
    {
        return cvf::BoundingBox();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimEclipseCase::allCellsBoundingBox() const
{
    if ( m_rigEclipseCase.notNull() && m_rigEclipseCase->mainGrid() )
    {
        return m_rigEclipseCase->mainGrid()->boundingBox();
    }
    else
    {
        return cvf::BoundingBox();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimEclipseCase::displayModelOffset() const
{
    if ( m_rigEclipseCase.notNull() && m_rigEclipseCase->mainGrid() )
    {
        return m_rigEclipseCase->mainGrid()->displayModelOffset();
    }
    else
    {
        return cvf::Vec3d::ZERO;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigCaseCellResultsData* RimEclipseCase::results( RiaDefines::PorosityModelType porosityModel )
{
    if ( m_rigEclipseCase.notNull() )
    {
        return m_rigEclipseCase->results( porosityModel );
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigCaseCellResultsData* RimEclipseCase::results( RiaDefines::PorosityModelType porosityModel ) const
{
    if ( m_rigEclipseCase.notNull() )
    {
        return m_rigEclipseCase->results( porosityModel );
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimReservoirCellResultsStorage* RimEclipseCase::resultsStorage( RiaDefines::PorosityModelType porosityModel )
{
    if ( porosityModel == RiaDefines::MATRIX_MODEL )
    {
        return m_matrixModelResults();
    }

    return m_fractureModelResults();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimReservoirCellResultsStorage* RimEclipseCase::resultsStorage( RiaDefines::PorosityModelType porosityModel ) const
{
    if ( porosityModel == RiaDefines::MATRIX_MODEL )
    {
        return m_matrixModelResults();
    }

    return m_fractureModelResults();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimEclipseCase::filesContainingFaults() const
{
    std::vector<QString> stdPathList;

    for ( auto& filePath : m_filesContainingFaults() )
    {
        stdPathList.push_back( filePath.path() );
    }

    return stdPathList;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::setFilesContainingFaults( const std::vector<QString>& pathStrings )
{
    std::vector<caf::FilePath> filePaths;
    for ( const auto& pathString : pathStrings )
    {
        filePaths.push_back( pathString );
    }

    m_filesContainingFaults = filePaths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseCase::ensureReservoirCaseIsOpen()
{
    if ( m_rigEclipseCase.notNull() )
    {
        return true;
    }

    return openReserviorCase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseCase::openReserviorCase()
{
    if ( !openEclipseGridFile() )
    {
        return false;
    }

    bool createPlaceholderEntries = true;
    if ( dynamic_cast<RimEclipseStatisticsCase*>( this ) )
    {
        // Never create placeholder entries for statistical cases. This does not make sense, and breaks the
        // logic for testing if data is present in RimEclipseStatisticsCase::hasComputedStatistics()
        createPlaceholderEntries = false;
    }

    if ( createPlaceholderEntries )
    {
        {
            RigCaseCellResultsData* results = this->results( RiaDefines::MATRIX_MODEL );
            if ( results )
            {
                results->createPlaceholderResultEntries();
                // After the placeholder result for combined transmissibility is created,
                // make sure the nnc transmissibilities can be addressed by this scalarResultIndex as well

                RigEclipseResultAddress combinedTransmissibilityResAddr( RiaDefines::STATIC_NATIVE,
                                                                         RiaDefines::combinedTransmissibilityResultName() );
                if ( results->hasResultEntry( combinedTransmissibilityResAddr ) )
                {
                    eclipseCaseData()->mainGrid()->nncData()->setEclResultAddress( RiaDefines::propertyNameCombTrans(),
                                                                                   combinedTransmissibilityResAddr );
                }

                RigEclipseResultAddress combinedWaterFluxResAddr( RiaDefines::DYNAMIC_NATIVE,
                                                                  RiaDefines::combinedWaterFluxResultName() );
                if ( results->hasResultEntry( combinedWaterFluxResAddr ) )
                {
                    eclipseCaseData()->mainGrid()->nncData()->setEclResultAddress( RiaDefines::propertyNameFluxWat(),
                                                                                   combinedWaterFluxResAddr );
                }

                RigEclipseResultAddress combinedOilFluxResAddr( RiaDefines::DYNAMIC_NATIVE,
                                                                RiaDefines::combinedOilFluxResultName() );
                if ( results->hasResultEntry( combinedOilFluxResAddr ) )
                {
                    eclipseCaseData()->mainGrid()->nncData()->setEclResultAddress( RiaDefines::propertyNameFluxOil(),
                                                                                   combinedOilFluxResAddr );
                }
                RigEclipseResultAddress combinedGasFluxResAddr( RiaDefines::DYNAMIC_NATIVE,
                                                                RiaDefines::combinedGasFluxResultName() );

                if ( results->hasResultEntry( combinedGasFluxResAddr ) )
                {
                    eclipseCaseData()->mainGrid()->nncData()->setEclResultAddress( RiaDefines::propertyNameFluxGas(),
                                                                                   combinedGasFluxResAddr );
                }
            }
        }

        {
            RigCaseCellResultsData* results = this->results( RiaDefines::FRACTURE_MODEL );
            if ( results )
            {
                results->createPlaceholderResultEntries();
            }
        }
    }

    createTimeStepFormatString();

    // Associate existing well paths with simulation wells
    RimProject* proj = RiaApplication::instance()->project();
    for ( const auto& oilField : proj->oilFields() )
    {
        for ( const auto& wellPath : oilField->wellPathCollection()->wellPaths() )
        {
            if ( !wellPath->isAssociatedWithSimulationWell() )
            {
                wellPath->tryAssociateWithSimulationWell();
            }
        }
    }

    // Update grids node
    {
        std::vector<RimGridCollection*> gridColls;
        descendantsIncludingThisOfType( gridColls );
        for ( RimGridCollection* gridCollection : gridColls )
        {
            gridCollection->syncFromMainEclipseGrid();
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Rim3dView*> RimEclipseCase::allSpecialViews() const
{
    std::vector<Rim3dView*> views;
    for ( RimEclipseView* view : reservoirViews )
    {
        views.push_back( view );
    }

    for ( RimEclipseContourMapView* view : m_contourMapCollection->views() )
    {
        views.push_back( view );
    }

    return views;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimEclipseCase::timeStepStrings() const
{
    QStringList stringList;

    const RigCaseCellResultsData* cellResultData = results( RiaDefines::MATRIX_MODEL );
    if ( cellResultData )
    {
        int timeStepCount = static_cast<int>( cellResultData->maxTimeStepCount() );
        for ( int i = 0; i < timeStepCount; i++ )
        {
            stringList += this->timeStepName( i );
        }
    }

    return stringList;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEclipseCase::timeStepName( int frameIdx ) const
{
    std::vector<QDateTime> timeStepDates = this->timeStepDates();
    if ( frameIdx < static_cast<int>( timeStepDates.size() ) )
    {
        QDateTime date = timeStepDates.at( frameIdx );

        return RiaQDateTimeTools::toStringUsingApplicationLocale( date, m_timeStepFormatString );
    }

    return QString( "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimEclipseCase::characteristicCellSize() const
{
    const RigEclipseCaseData* rigEclipseCase = eclipseCaseData();
    if ( rigEclipseCase )
    {
        return rigEclipseCase->mainGrid()->characteristicIJCellSize();
    }

    return 10.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString> RimEclipseCase::sortedSimWellNames() const
{
    std::set<QString> sortedWellNames;
    if ( eclipseCaseData() )
    {
        const cvf::Collection<RigSimWellData>& simWellData = eclipseCaseData()->wellResults();

        for ( size_t wIdx = 0; wIdx < simWellData.size(); ++wIdx )
        {
            sortedWellNames.insert( simWellData[wIdx]->m_wellName );
        }
    }
    return sortedWellNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RimEclipseCase::timeStepDates() const
{
    if ( results( RiaDefines::MATRIX_MODEL ) )
    {
        return results( RiaDefines::MATRIX_MODEL )->timeStepDates();
    }
    return std::vector<QDateTime>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseCase::importAsciiInputProperties( const QStringList& fileNames )
{
    return false;
}
