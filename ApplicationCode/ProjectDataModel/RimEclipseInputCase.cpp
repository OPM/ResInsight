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

#include "RimEclipseInputCase.h"

#include "RiaFieldHandleTools.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "RifEclipseInputFileTools.h"
#include "RifEclipseInputPropertyLoader.h"
#include "RifReaderEclipseInput.h"
#include "RifReaderInterface.h"
#include "RifReaderMockModel.h"
#include "RifReaderSettings.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"

#include "RiaDefines.h"
#include "RimEclipseInputProperty.h"
#include "RimEclipseInputPropertyCollection.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimTools.h"

#include "cafProgressInfo.h"

#include <QDir>
#include <QFileInfo>

CAF_PDM_SOURCE_INIT( RimEclipseInputCase, "RimInputReservoir" );
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseInputCase::RimEclipseInputCase()
    : RimEclipseCase()
{
    CAF_PDM_InitObject( "RimInputCase", ":/EclipseInput48x48.png", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_gridFileName, "GridFileName", "Case File Name", "", "", "" );
    m_gridFileName.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_additionalFiles, "AdditionalFileNamesProxy", "Additional Files", "", "", "" );
    m_additionalFiles.registerGetMethod( this, &RimEclipseInputCase::additionalFiles );
    m_additionalFiles.uiCapability()->setUiReadOnly( true );
    m_additionalFiles.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitFieldNoDefault( &m_additionalFilenames_OBSOLETE, "AdditionalFileNames", "Additional Files", "", "", "" );
    m_additionalFilenames_OBSOLETE.uiCapability()->setUiReadOnly( true );
    RiaFieldhandleTools::disableWriteAndSetFieldHidden( &m_additionalFilenames_OBSOLETE );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseInputCase::~RimEclipseInputCase()
{
}

//--------------------------------------------------------------------------------------------------
/// Import ascii properties. If no grid data has been read, it will first find the possible
/// grid data among the files then read all supported properties from the files matching the grid
//--------------------------------------------------------------------------------------------------
bool RimEclipseInputCase::importAsciiInputProperties( const QStringList& fileNames )
{
    return openDataFileSet( fileNames );
}

//--------------------------------------------------------------------------------------------------
/// Open the supplied file set. If no grid data has been read, it will first find the possible
/// grid data among the files then read all supported properties from the files matching the grid
//--------------------------------------------------------------------------------------------------
bool RimEclipseInputCase::openDataFileSet( const QStringList& fileNames )
{
    if ( fileNames.contains( RiaDefines::mockModelBasicInputCase() ) )
    {
        cvf::ref<RifReaderInterface> readerInterface = this->createMockModel( fileNames[0] );
        results( RiaDefines::MATRIX_MODEL )->setReaderInterface( readerInterface.p() );
        results( RiaDefines::FRACTURE_MODEL )->setReaderInterface( readerInterface.p() );

        eclipseCaseData()->activeCellInfo( RiaDefines::MATRIX_MODEL )->computeDerivedData();
        eclipseCaseData()->activeCellInfo( RiaDefines::FRACTURE_MODEL )->computeDerivedData();

        QFileInfo gridFileName( fileNames[0] );
        QString   caseName        = gridFileName.completeBaseName();
        this->caseUserDescription = caseName;

        computeCachedData();

        return true;
    }

    if ( this->eclipseCaseData() == nullptr )
    {
        this->setReservoirData( new RigEclipseCaseData( this ) );
    }

    bool importFaults = RiaApplication::instance()->preferences()->readerSettings()->importFaults();

    std::vector<QString> allErrorMessages;

    // First find and read the grid data
    if ( this->eclipseCaseData()->mainGrid()->gridPointDimensions() == cvf::Vec3st( 0, 0, 0 ) )
    {
        for ( int i = 0; i < fileNames.size(); i++ )
        {
            QString errorMessages;
            if ( RifEclipseInputFileTools::openGridFile( fileNames[i], this->eclipseCaseData(), importFaults, &errorMessages ) )
            {
                m_gridFileName = fileNames[i];

                QFileInfo gridFileName( fileNames[i] );
                QString   caseName = gridFileName.completeBaseName();

                this->caseUserDescription = caseName;

                this->eclipseCaseData()->mainGrid()->setFlipAxis( m_flipXAxis, m_flipYAxis );

                computeCachedData();

                break;
            }
            else
            {
                allErrorMessages.push_back( errorMessages );
            }
        }
    }

    if ( this->eclipseCaseData()->mainGrid()->gridPointDimensions() == cvf::Vec3st( 0, 0, 0 ) )
    {
        if ( !allErrorMessages.empty() )
        {
            for ( QString errorMessages : allErrorMessages )
            {
                RiaLogging::error( errorMessages );
            }
        }
        return false; // No grid present
    }

    std::vector<QString> filesToRead;
    for ( const QString& filename : fileNames )
    {
        bool exists = false;
        for ( const QString& currentFileName : additionalFiles() )
        {
            if ( filename == currentFileName )
            {
                exists = true;
                break;
            }
        }
        if ( !exists )
        {
            filesToRead.push_back( filename );
        }
    }

    RifEclipseInputPropertyLoader::readInputPropertiesFromFiles( m_inputPropertyCollection,
                                                                 this->eclipseCaseData(),
                                                                 importFaults,
                                                                 filesToRead );
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseInputCase::openEclipseGridFile()
{
    // Early exit if reservoir data is created
    if ( this->eclipseCaseData() == nullptr )
    {
        cvf::ref<RifReaderInterface> readerInterface;

        if ( m_gridFileName().path().contains( RiaDefines::mockModelBasicInputCase() ) )
        {
            readerInterface = this->createMockModel( this->m_gridFileName().path() );
        }
        else
        {
            readerInterface = new RifReaderEclipseInput;

            cvf::ref<RigEclipseCaseData> eclipseCase = new RigEclipseCaseData( this );
            if ( !readerInterface->open( m_gridFileName().path(), eclipseCase.p() ) )
            {
                return false;
            }

            this->setReservoirData( eclipseCase.p() );
        }

        CVF_ASSERT( this->eclipseCaseData() );
        CVF_ASSERT( readerInterface.notNull() );

        results( RiaDefines::MATRIX_MODEL )->setReaderInterface( readerInterface.p() );
        results( RiaDefines::FRACTURE_MODEL )->setReaderInterface( readerInterface.p() );

        this->eclipseCaseData()->mainGrid()->setFlipAxis( m_flipXAxis, m_flipYAxis );

        computeCachedData();
        loadAndSyncronizeInputProperties();
    }

    RiaApplication* app = RiaApplication::instance();
    if ( app->preferences()->autocomputeDepthRelatedProperties )
    {
        results( RiaDefines::MATRIX_MODEL )->computeDepthRelatedResults();
        results( RiaDefines::FRACTURE_MODEL )->computeDepthRelatedResults();
    }

    results( RiaDefines::MATRIX_MODEL )->computeCellVolumes();

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseInputCase::reloadEclipseGridFile()
{
    setReservoirData( nullptr );
    openReserviorCase();
}

//--------------------------------------------------------------------------------------------------
/// Loads input property data from the gridFile and additional files
/// Creates new InputProperties if necessary, and flags the unused ones as obsolete
//--------------------------------------------------------------------------------------------------
void RimEclipseInputCase::loadAndSyncronizeInputProperties()
{
    // Make sure we actually have reservoir data

    CVF_ASSERT( this->eclipseCaseData() );
    CVF_ASSERT( this->eclipseCaseData()->mainGrid()->gridPointDimensions() != cvf::Vec3st( 0, 0, 0 ) );

    // Then read the properties from all the files referenced by the InputReservoir

    std::vector<QString> filenames;
    for ( const QString& fileName : additionalFiles() )
    {
        filenames.push_back( fileName );
    }
    filenames.push_back( m_gridFileName().path() );

    RifEclipseInputPropertyLoader::loadAndSyncronizeInputProperties( inputPropertyCollection(), eclipseCaseData(), filenames );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RifReaderInterface> RimEclipseInputCase::createMockModel( QString modelName )
{
    cvf::ref<RigEclipseCaseData> reservoir         = new RigEclipseCaseData( this );
    cvf::ref<RifReaderMockModel> mockFileInterface = new RifReaderMockModel;

    if ( modelName == RiaDefines::mockModelBasicInputCase() )
    {
        m_gridFileName = modelName;

        // Create the mock file interface and and RigSerervoir and set them up.
        mockFileInterface->setWorldCoordinates( cvf::Vec3d( 10, 10, 10 ), cvf::Vec3d( 20, 20, 20 ) );
        mockFileInterface->setGridPointDimensions( cvf::Vec3st( 4, 5, 6 ) );
        mockFileInterface->addLocalGridRefinement( cvf::Vec3st( 0, 2, 2 ), cvf::Vec3st( 0, 2, 2 ), cvf::Vec3st( 3, 3, 3 ) );
        mockFileInterface->setResultInfo( 3, 10 );

        mockFileInterface->open( "", reservoir.p() );
        {
            // size_t idx = reservoir->mainGrid()->cellIndexFromIJK(1, 3, 4);

            // TODO: Rewrite active cell info in mock models
            // reservoir->mainGrid()->cell(idx).setActiveIndexInMatrixModel(cvf::UNDEFINED_SIZE_T);
        }

        {
            // size_t idx = reservoir->mainGrid()->cellIndexFromIJK(2, 2, 3);

            // TODO: Rewrite active cell info in mock models
            // reservoir->mainGrid()->cell(idx).setActiveIndexInMatrixModel(cvf::UNDEFINED_SIZE_T);
        }

        // Add a property
        RimEclipseInputProperty* inputProperty = new RimEclipseInputProperty;
        inputProperty->resultName              = "PORO";
        inputProperty->eclipseKeyword          = "PORO";
        inputProperty->fileName                = QString( "PORO.prop" );
        m_inputPropertyCollection->inputProperties.push_back( inputProperty );
    }

    this->setReservoirData( reservoir.p() );

    return mockFileInterface.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseInputCase::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &caseUserDescription );
    uiOrdering.add( &caseId );
    uiOrdering.add( &m_gridFileName );
    uiOrdering.add( &m_additionalFiles );

    auto group = uiOrdering.addNewGroup( "Case Options" );
    group->add( &m_activeFormationNames );
    group->add( &m_flipXAxis );
    group->add( &m_flipYAxis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEclipseInputCase::locationOnDisc() const
{
    if ( m_gridFileName().path().isEmpty() ) return QString();

    QFileInfo fi( m_gridFileName().path() );
    return fi.absolutePath();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseInputCase::updateFilePathsFromProjectPath( const QString& newProjectPath, const QString& oldProjectPath )
{
    // bool                 foundFile = false;
    // std::vector<QString> searchedPaths;

    // m_gridFileName = RimTools::relocateFile( m_gridFileName().path(), newProjectPath, oldProjectPath, &foundFile,
    // &searchedPaths );

    // for ( RimEclipseInputProperty* inputProperty : m_inputPropertyCollection()->inputProperties() )
    //{
    //    inputProperty->fileName = RimTools::relocateFile( inputProperty->fileName,
    //                                                      newProjectPath,
    //                                                      oldProjectPath,
    //                                                      &foundFile,
    //                                                      &searchedPaths );
    //}
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseInputCase::updateAdditionalFileFolder( const QString& newFolder )
{
    QDir newDir( newFolder );
    for ( RimEclipseInputProperty* inputProperty : m_inputPropertyCollection()->inputProperties() )
    {
        if ( inputProperty->fileName == m_gridFileName().path() ) continue;

        QFileInfo oldFilePath( inputProperty->fileName().path() );
        QFileInfo newFilePath( newDir, oldFilePath.fileName() );
        inputProperty->fileName = newFilePath.absoluteFilePath();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimEclipseInputCase::additionalFiles() const
{
    std::vector<QString> additionalFiles;
    for ( const RimEclipseInputProperty* inputProperty : m_inputPropertyCollection()->inputProperties() )
    {
        if ( inputProperty->fileName == m_gridFileName().path() ) continue;

        additionalFiles.push_back( inputProperty->fileName().path() );
    }

    return additionalFiles;
}
