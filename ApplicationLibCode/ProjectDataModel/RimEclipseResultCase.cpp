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

#include "RimEclipseResultCase.h"

#include "RiaApplication.h"
#include "RiaFieldHandleTools.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"
#include "RiaRegressionTestRunner.h"
#include "RiaResultNames.h"

#include "RicfCommandObject.h"

#include "RifEclipseOutputFileTools.h"
#include "RifInputPropertyLoader.h"
#include "RifReaderEclipseOutput.h"
#include "RifReaderEclipseRft.h"
#include "RifReaderMockModel.h"
#include "RifReaderOpmCommon.h"
#include "RifReaderOpmRft.h"
#include "RifReaderSettings.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigFlowDiagSolverInterface.h"
#include "RigMainGrid.h"

#include "RimDialogData.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseInputProperty.h"
#include "RimEclipseInputPropertyCollection.h"
#include "RimEclipseView.h"
#include "RimFlowDiagSolution.h"
#include "RimMockModelSettings.h"
#include "RimProject.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimTimeStepFilter.h"
#include "RimTools.h"

#include "cafPdmUiCheckBoxAndTextEditor.h"
#include "cafPdmUiFilePathEditor.h"
#include "cafPdmUiPropertyViewDialog.h"
#include "cafProgressInfo.h"
#include "cafUtils.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>

#include <fstream>
#include <string>

CAF_PDM_SOURCE_INIT( RimEclipseResultCase, "EclipseCase" );
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseResultCase::RimEclipseResultCase()
    : m_gridAndWellDataIsReadFromFile( false )
    , m_activeCellInfoIsReadFromFile( false )
    , m_useOpmRftReader( true )
    , m_rftDataIsReadFromFile( false )
{
    CAF_PDM_InitScriptableObject( "Eclipse Case", ":/Case48x48.png", "", "The Regular Eclipse Results Case" );

    CAF_PDM_InitFieldNoDefault( &m_unitSystem, "UnitSystem", "Unit System" );
    m_unitSystem.registerGetMethod( RimProject::current(), &RimProject::commonUnitSystemForAllCases );
    m_unitSystem.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_flowDiagSolutions, "FlowDiagSolutions", "Flow Diagnostics Solutions" );
    m_flowDiagSolutions.uiCapability()->setUiTreeChildrenHidden( true );

    m_flipXAxis.xmlCapability()->setIOWritable( true );
    m_flipYAxis.xmlCapability()->setIOWritable( true );

    CAF_PDM_InitFieldNoDefault( &m_sourSimFileName, "SourSimFileName", "SourSim File Name" );
    m_sourSimFileName.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
#ifndef USE_HDF5
    m_sourSimFileName.uiCapability()->setUiHidden( true );
#endif

    CAF_PDM_InitField( &m_mswMergeThreshold, "MswMergeThreshold", std::make_pair( false, 3 ), "MSW Short Well Merge Threshold" );
    m_mswMergeThreshold.uiCapability()->setUiEditorTypeName( caf::PdmUiCheckBoxAndTextEditor::uiEditorTypeName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultCase::openEclipseGridFile()
{
    return importGridAndResultMetaData( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultCase::importGridAndResultMetaData( bool showTimeStepFilter )
{
    // Early exit if data is already read
    // Make sure that the progress info dialog is created after the return statement. If created before, the progress
    // dialog triggers a redraw with incomplete geometry data and causes a crash
    if ( m_gridAndWellDataIsReadFromFile ) return true;

    caf::ProgressInfo progInfo( 50, "Reading Eclipse Grid File" );

    progInfo.setProgressDescription( "Open Grid File" );
    progInfo.setNextProgressIncrement( 48 );

    cvf::ref<RifReaderInterface> readerInterface;

    if ( gridFileName().contains( "Result Mock Debug Model" ) )
    {
        readerInterface = createMockModel( gridFileName() );
    }
    else
    {
        if ( !caf::Utils::fileExists( gridFileName() ) )
        {
            return false;
        }

        auto defaultReader = RiaPreferences::current()->gridModelReader();

        if ( defaultReader == RiaDefines::GridModelReader::RESDATA )
        {
            auto readerEclipseOutput = new RifReaderEclipseOutput();

            cvf::ref<RifEclipseRestartDataAccess> restartDataAccess = RifEclipseOutputFileTools::createDynamicResultAccess( gridFileName() );

            {
                std::vector<QDateTime> timeSteps;
                std::vector<double>    daysSinceSimulationStart;

                if ( restartDataAccess.notNull() )
                {
                    restartDataAccess->timeSteps( &timeSteps, &daysSinceSimulationStart );
                }
                m_timeStepFilter->setTimeStepsFromFile( timeSteps );
            }

            if ( showTimeStepFilter )
            {
                caf::PdmUiPropertyViewDialog propertyDialog( nullptr,
                                                             m_timeStepFilter,
                                                             "Time Step Filter",
                                                             "",
                                                             QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
                propertyDialog.resize( QSize( 400, 400 ) );

                // Push arrow cursor onto the cursor stack so it takes over from the wait cursor.
                QApplication::setOverrideCursor( QCursor( Qt::ArrowCursor ) );
                // Show GUI to select time steps
                int dialogReturnValue = propertyDialog.exec();
                // Pop arrow cursor off the cursor stack so that the previous (wait) cursor takes over.
                QApplication::restoreOverrideCursor();

                if ( dialogReturnValue != QDialog::Accepted )
                {
                    return false;
                }
                m_timeStepFilter->updateFilteredTimeStepsFromUi();
            }

            readerEclipseOutput->setFileDataAccess( restartDataAccess.p() );
            readerEclipseOutput->setTimeStepFilter( m_timeStepFilter->filteredTimeSteps() );

            readerInterface = readerEclipseOutput;
        }
        else
        {
            readerInterface = new RifReaderOpmCommon;
        }

        readerInterface->setFilenamesWithFaults( filesContainingFaults() );
        readerInterface->setReaderSettings( m_readerSettings );

        cvf::ref<RigEclipseCaseData> eclipseCase = new RigEclipseCaseData( this );
        if ( !readerInterface->open( gridFileName(), eclipseCase.p() ) )
        {
            return false;
        }

        setFilesContainingFaults( readerInterface->filenamesWithFaults() );

        setReservoirData( eclipseCase.p() );
    }

    results( RiaDefines::PorosityModelType::MATRIX_MODEL )->setReaderInterface( readerInterface.p() );
    results( RiaDefines::PorosityModelType::FRACTURE_MODEL )->setReaderInterface( readerInterface.p() );

    progInfo.incrementProgress();

    m_flowDagSolverInterface = new RigFlowDiagSolverInterface( this );

    CVF_ASSERT( eclipseCaseData() );
    CVF_ASSERT( readerInterface.notNull() );

    progInfo.setProgressDescription( "Computing Case Cache" );
    computeCachedData();
    loadAndSynchronizeInputProperties( false );

    m_gridAndWellDataIsReadFromFile = true;
    m_activeCellInfoIsReadFromFile  = true;

    ensureRftDataIsImported();

    if ( m_flowDiagSolutions.empty() )
    {
        m_flowDiagSolutions.push_back( new RimFlowDiagSolution() );
    }

    if ( !m_sourSimFileName().path().isEmpty() )
    {
        auto* outReader = dynamic_cast<RifReaderEclipseOutput*>( readerInterface.p() );
        outReader->setHdf5FileName( m_sourSimFileName().path() );
    }

    if ( RiaPreferences::current()->autocomputeDepthRelatedProperties )
    {
        results( RiaDefines::PorosityModelType::MATRIX_MODEL )->computeDepthRelatedResults();
        results( RiaDefines::PorosityModelType::FRACTURE_MODEL )->computeDepthRelatedResults();
    }

    results( RiaDefines::PorosityModelType::MATRIX_MODEL )->computeCellVolumes();

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultCase::importAsciiInputProperties( const QStringList& fileNames )
{
    bool importFaults = false;
    RifInputPropertyLoader::loadAndSynchronizeInputProperties( m_inputPropertyCollection,
                                                               eclipseCaseData(),
                                                               std::vector<QString>( fileNames.begin(), fileNames.end() ),
                                                               importFaults );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultCase::reloadEclipseGridFile()
{
    m_gridAndWellDataIsReadFromFile = false;
    m_activeCellInfoIsReadFromFile  = false;
    setReservoirData( nullptr );
    openReserviorCase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultCase::openAndReadActiveCellData( RigEclipseCaseData* mainEclipseCase )
{
    // Early exit if data is already read
    if ( m_activeCellInfoIsReadFromFile ) return true;

    cvf::ref<RifReaderInterface> readerInterface;
    if ( gridFileName().contains( "Result Mock Debug Model" ) )
    {
        readerInterface = createMockModel( gridFileName() );
    }
    else
    {
        if ( !caf::Utils::fileExists( gridFileName() ) )
        {
            return false;
        }

        cvf::ref<RigEclipseCaseData> eclipseCase = new RigEclipseCaseData( this );

        CVF_ASSERT( mainEclipseCase && mainEclipseCase->mainGrid() );
        eclipseCase->setMainGrid( mainEclipseCase->mainGrid() );

        std::vector<QDateTime> timeStepDates = mainEclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->timeStepDates();
        cvf::ref<RifReaderEclipseOutput> readerEclipseOutput = new RifReaderEclipseOutput;
        if ( !readerEclipseOutput->openAndReadActiveCellData( gridFileName(), timeStepDates, eclipseCase.p() ) )
        {
            return false;
        }

        setReservoirData( eclipseCase.p() );

        readerInterface = readerEclipseOutput;
    }

    results( RiaDefines::PorosityModelType::MATRIX_MODEL )->setReaderInterface( readerInterface.p() );
    results( RiaDefines::PorosityModelType::FRACTURE_MODEL )->setReaderInterface( readerInterface.p() );

    CVF_ASSERT( eclipseCaseData() );
    CVF_ASSERT( readerInterface.notNull() );

    computeActiveCellsBoundingBox();

    m_activeCellInfoIsReadFromFile = true;

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::AppEnum<RiaDefines::EclipseUnitSystem> RimEclipseResultCase::unitSystem()
{
    return m_unitSystem();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultCase::loadAndUpdateSourSimData()
{
    if ( !results( RiaDefines::PorosityModelType::MATRIX_MODEL ) ) return;

    results( RiaDefines::PorosityModelType::MATRIX_MODEL )->setHdf5Filename( m_sourSimFileName().path() );

    if ( !hasSourSimFile() )
    {
        // Deselect SourSimRL cell results
        for ( Rim3dView* view : views() )
        {
            auto* eclipseView = dynamic_cast<RimEclipseView*>( view );
            if ( eclipseView != nullptr )
            {
                if ( eclipseView->cellResult()->resultType() == RiaDefines::ResultCatType::SOURSIMRL )
                {
                    eclipseView->cellResult()->setResultType( RiaDefines::ResultCatType::DYNAMIC_NATIVE );
                    eclipseView->cellResult()->setResultVariable( RiaResultNames::soil() );
                    eclipseView->loadDataAndUpdate();
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultCase::ensureRftDataIsImported()
{
    if ( m_rftDataIsReadFromFile ) return;

    QFileInfo eclipseCaseFileInfo( gridFileName() );
    QString   rftFileName = eclipseCaseFileInfo.path() + "/" + eclipseCaseFileInfo.completeBaseName() + ".RFT";
    QFileInfo rftFileInfo( rftFileName );

    if ( rftFileInfo.exists() )
    {
        if ( m_useOpmRftReader )
        {
            m_readerOpmRft = new RifReaderOpmRft( rftFileInfo.filePath() );
        }
        else
        {
            m_readerEclipseRft = new RifReaderEclipseRft( rftFileInfo.filePath() );
        }
    }

    m_rftDataIsReadFromFile = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RifReaderInterface> RimEclipseResultCase::createMockModel( QString modelName )
{
    cvf::ref<RifReaderMockModel> mockFileInterface = new RifReaderMockModel;
    cvf::ref<RigEclipseCaseData> reservoir         = new RigEclipseCaseData( this );

    if ( modelName == RiaDefines::mockModelBasic() )
    {
        // Create the mock file interface and and RigSerervoir and set them up.
        mockFileInterface->setWorldCoordinates( cvf::Vec3d( 10, 10, 10 ), cvf::Vec3d( 20, 20, 20 ) );
        mockFileInterface->setGridPointDimensions( cvf::Vec3st( 4, 5, 6 ) );
        mockFileInterface->addLocalGridRefinement( cvf::Vec3st( 0, 2, 2 ), cvf::Vec3st( 0, 2, 2 ), cvf::Vec3st( 3, 3, 3 ) );
        mockFileInterface->enableWellData( false );

        mockFileInterface->open( "", reservoir.p() );
    }
    else if ( modelName == RiaDefines::mockModelBasicWithResults() )
    {
        mockFileInterface->setWorldCoordinates( cvf::Vec3d( 10, 10, 10 ), cvf::Vec3d( -20, -20, -20 ) );
        mockFileInterface->setGridPointDimensions( cvf::Vec3st( 5, 10, 20 ) );
        mockFileInterface->addLocalGridRefinement( cvf::Vec3st( 0, 3, 3 ), cvf::Vec3st( 1, 4, 9 ), cvf::Vec3st( 2, 2, 2 ) );
        mockFileInterface->setResultInfo( 3, 10 );

        mockFileInterface->open( "", reservoir.p() );

        // Make a fault
        cvf::Vec3d& tmp = reservoir->mainGrid()->nodes()[1];
        tmp += cvf::Vec3d( 1, 0, 0 );
    }
    else if ( modelName == RiaDefines::mockModelLargeWithResults() )
    {
        double startX = 0;
        double startY = 0;
        double startZ = 0;

        double widthX = 6000;
        double widthY = 12000;
        double widthZ = 500;

        double offsetX = 0;
        double offsetY = 0;
        double offsetZ = 0;

        // Test code to simulate UTM coordinates
        offsetX = 400000;
        offsetY = 6000000;
        offsetZ = 0;

        mockFileInterface->setWorldCoordinates( cvf::Vec3d( startX + offsetX, startY + offsetY, startZ + offsetZ ),
                                                cvf::Vec3d( startX + widthX + offsetX, startY + widthY + offsetY, startZ + widthZ + offsetZ ) );
        mockFileInterface->setGridPointDimensions( cvf::Vec3st( 50, 100, 200 ) );
        mockFileInterface->addLocalGridRefinement( cvf::Vec3st( 0, 30, 30 ), cvf::Vec3st( 1, 40, 90 ), cvf::Vec3st( 2, 2, 2 ) );
        mockFileInterface->setResultInfo( 3, 10 );

        mockFileInterface->open( "", reservoir.p() );
    }
    else if ( modelName == RiaDefines::mockModelCustomized() )
    {
        QApplication::setOverrideCursor( QCursor( Qt::ArrowCursor ) );

        RimMockModelSettings* mockModelSettings = RimProject::current()->dialogData()->mockModelSettings();

        if ( !RiaRegressionTestRunner::instance()->isRunningRegressionTests() )
        {
            caf::PdmUiPropertyViewDialog propertyDialog( nullptr, mockModelSettings, "Customize Mock Model", "" );
            if ( propertyDialog.exec() == QDialog::Accepted )
            {
            }
        }

        {
            double startX = 0;
            double startY = 0;
            double startZ = 0;

            double widthX = 6000;
            double widthY = 12000;
            double widthZ = 500;

            // Test code to simulate UTM coordinates
            double offsetX = 400000;
            double offsetY = 6000000;
            double offsetZ = 0;

            mockFileInterface->setWorldCoordinates( cvf::Vec3d( startX + offsetX, startY + offsetY, startZ + offsetZ ),
                                                    cvf::Vec3d( startX + widthX + offsetX, startY + widthY + offsetY, startZ + widthZ + offsetZ ) );
            mockFileInterface->setGridPointDimensions(
                cvf::Vec3st( mockModelSettings->cellCountX + 1, mockModelSettings->cellCountY + 1, mockModelSettings->cellCountZ + 1 ) );
            mockFileInterface->setResultInfo( mockModelSettings->resultCount, mockModelSettings->timeStepCount );
            mockFileInterface->enableWellData( false );

            mockFileInterface->open( "", reservoir.p() );
        }

        QApplication::restoreOverrideCursor();
    }

    setReservoirData( reservoir.p() );

    return mockFileInterface.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseResultCase::~RimEclipseResultCase()
{
    // Disconnect all comparison views. In debug build on Windows, a crash occurs. The comparison view is also set to zero in the destructor
    // of Rim3dView()
    for ( auto v : reservoirViews() )
    {
        if ( v ) v->setComparisonView( nullptr );
    }

    m_flowDiagSolutions.deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEclipseResultCase::locationOnDisc() const
{
    QFileInfo fi( gridFileName() );
    return fi.absolutePath();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultCase::readGridDimensions( std::vector<std::vector<int>>& gridDimensions )
{
    RifEclipseOutputFileTools::readGridDimensions( gridFileName(), gridDimensions );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFlowDiagSolution* RimEclipseResultCase::defaultFlowDiagSolution()
{
    if ( !m_flowDiagSolutions.empty() )
    {
        return m_flowDiagSolutions[0];
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimFlowDiagSolution*> RimEclipseResultCase::flowDiagSolutions()
{
    std::vector<RimFlowDiagSolution*> flowSols;
    for ( const caf::PdmPointer<RimFlowDiagSolution>& fsol : m_flowDiagSolutions )
    {
        flowSols.push_back( fsol.p() );
    }

    return flowSols;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFlowDiagSolverInterface* RimEclipseResultCase::flowDiagSolverInterface()
{
    return m_flowDagSolverInterface.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderRftInterface* RimEclipseResultCase::rftReader()
{
    ensureRftDataIsImported();

    if ( m_useOpmRftReader ) return m_readerOpmRft.p();

    return m_readerEclipseRft.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimEclipseResultCase::mswMergeThreshold() const
{
    // This value is used in RigMswCenterLineCalculator::calculateMswWellPipeGeometry

    if ( m_mswMergeThreshold().first )
    {
        return m_mswMergeThreshold().second;
    }

    return 4;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultCase::setCaseInfo( const QString& userDescription, const QString& fileName )
{
    setCaseUserDescription( userDescription );
    setGridFileName( fileName );

    RimProject* proj = RimProject::current();
    proj->assignCaseIdToCase( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultCase::setSourSimFileName( const QString& fileName )
{
    m_sourSimFileName = fileName;

    loadAndUpdateSourSimData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultCase::hasSourSimFile()
{
    return !m_sourSimFileName().path().isEmpty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultCase::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_caseUserDescription );
    uiOrdering.add( &m_displayNameOption );
    uiOrdering.add( &m_caseId );
    uiOrdering.add( &m_caseFileName );
    uiOrdering.add( &m_unitSystem );

    auto group = uiOrdering.addNewGroup( "Case Options" );
    group->add( &m_activeFormationNames );
    group->add( &m_flipXAxis );
    group->add( &m_flipYAxis );
    group->add( &m_mswMergeThreshold );

    if ( eclipseCaseData() && eclipseCaseData()->results( RiaDefines::PorosityModelType::MATRIX_MODEL ) &&
         eclipseCaseData()->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->maxTimeStepCount() > 0 )
    {
        auto group1 = uiOrdering.addNewGroup( "Time Step Filter" );
        group1->setCollapsedByDefault();
        m_timeStepFilter->uiOrdering( uiConfigName, *group1 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultCase::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_sourSimFileName )
    {
        loadAndUpdateSourSimData();
    }

    if ( changedField == &m_mswMergeThreshold )
    {
        for ( auto resView : reservoirViews() )
        {
            resView->scheduleSimWellGeometryRegen();
            resView->scheduleCreateDisplayModelAndRedraw();
        }
    }

    return RimEclipseCase::fieldChangedByUi( changedField, oldValue, newValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultCase::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_sourSimFileName )
    {
        auto* myAttr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->m_fileSelectionFilter = "SourSim (*.sourres)";
            myAttr->m_defaultPath         = QFileInfo( gridFileName() ).absolutePath();
        }
    }
}
