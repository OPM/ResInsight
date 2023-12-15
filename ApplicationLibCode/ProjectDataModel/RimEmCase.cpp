/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Ceetron Solutions AS
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

#include "RimEmCase.h"

#include "RiaDefines.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "RifInputPropertyLoader.h"
#include "RifRoffFileTools.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"

#include "RimEclipseInputProperty.h"
#include "RimEclipseInputPropertyCollection.h"
#include "RimReservoirCellResultsStorage.h"

#include "cafPdmObjectScriptingCapability.h"
#include "cafProgressInfo.h"

#include "H5Cpp.h"

#include <QDir>
#include <QFileInfo>

CAF_PDM_SOURCE_INIT( RimEmCase, "RimEmCase" );
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEmCase::RimEmCase()
    : RimEclipseCase()
{
    CAF_PDM_InitScriptableObject( "RimEmCase", ":/EclipseInput48x48.png" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEmCase::~RimEmCase()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEmCase::openEclipseGridFile()
{
    if ( eclipseCaseData() )
    {
        // Early exit if reservoir data is created
        return true;
    }

    setReservoirData( new RigEclipseCaseData( this ) );

    QString fileName = gridFileName();

    // First find and read the grid data
    if ( eclipseCaseData()->mainGrid()->gridPointDimensions() == cvf::Vec3st( 0, 0, 0 ) )
    {
        try
        {
            H5::Exception::dontPrint(); // Turn off auto-printing of failures to handle the errors appropriately

            H5::H5File mainFile( fileName.toStdString().c_str(),
                                 H5F_ACC_RDONLY ); // initial date part is an attribute of SourSimRL main file
        }
        catch ( ... )
        {
        }

        /*
    QString errorMessages;
    if ( RifRoffFileTools::openGridFile( fileName, eclipseCaseData(), &errorMessages ) )
    {
        QFileInfo gridFileInfo( fileName );
        QString   caseName = gridFileInfo.completeBaseName();

        setCaseUserDescription( caseName );
        eclipseCaseData()->mainGrid()->setFlipAxis( m_flipXAxis, m_flipYAxis );
        computeCachedData();
    }
    else
    {
        RiaLogging::error( errorMessages );
        return false;
    }
*/
    }

    results( RiaDefines::PorosityModelType::MATRIX_MODEL )->createPlaceholderResultEntries();

    if ( RiaPreferences::current()->autocomputeDepthRelatedProperties )
    {
        results( RiaDefines::PorosityModelType::MATRIX_MODEL )->computeDepthRelatedResults();
        results( RiaDefines::PorosityModelType::FRACTURE_MODEL )->computeDepthRelatedResults();
    }

    results( RiaDefines::PorosityModelType::MATRIX_MODEL )->computeCellVolumes();

    // Read properties from grid file
    RifRoffFileTools::createInputProperties( fileName, eclipseCaseData() );

    // Read properties from input property collection
    loadAndSynchronizeInputProperties( false );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEmCase::reloadEclipseGridFile()
{
    setReservoirData( nullptr );
    openReserviorCase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEmCase::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_caseUserDescription );
    uiOrdering.add( &m_displayNameOption );
    uiOrdering.add( &m_caseId );
    uiOrdering.add( &m_caseFileName );

    auto group = uiOrdering.addNewGroup( "Case Options" );
    group->add( &m_activeFormationNames );
    group->add( &m_flipXAxis );
    group->add( &m_flipYAxis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEmCase::locationOnDisc() const
{
    if ( gridFileName().isEmpty() ) return QString();

    QFileInfo fi( gridFileName() );
    return fi.absolutePath();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEmCase::importAsciiInputProperties( const QStringList& fileNames )
{
    bool importFaults = false;
    RifInputPropertyLoader::loadAndSynchronizeInputProperties( m_inputPropertyCollection,
                                                               eclipseCaseData(),
                                                               std::vector<QString>( fileNames.begin(), fileNames.end() ),
                                                               importFaults );

    return true;
}
