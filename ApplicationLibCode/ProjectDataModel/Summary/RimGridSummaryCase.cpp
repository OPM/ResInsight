/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RimGridSummaryCase.h"

#include "RiaSummaryTools.h"

#include "RicfCommandObject.h"

#include "RifSummaryReaderInterface.h"

#include "RimEclipseCase.h"
#include "RimFileSummaryCase.h"
#include "RimProject.h"
#include "RimSummaryCaseMainCollection.h"

#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmPtrField.h"

#include <QFileInfo>

//==================================================================================================
//
//
//
//==================================================================================================

CAF_PDM_SOURCE_INIT( RimGridSummaryCase_obsolete, "GridSummaryCase" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridSummaryCase_obsolete::RimGridSummaryCase_obsolete()
{
    CAF_PDM_InitScriptableObject( "Grid Summary Case", ":/SummaryCases16x16.png", "", "A Summary Case based on extracting grid data." );
    CAF_PDM_InitFieldNoDefault( &m_eclipseCase, "Associated3DCase", "Eclipse Case" );
    m_eclipseCase.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_cachedCaseName, "CachedCasename", "Case Name" );
    m_cachedCaseName.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_eclipseGridFileName, "Associated3DCaseGridFileName", "Grid File Name" );
    m_eclipseGridFileName.registerGetMethod( this, &RimGridSummaryCase_obsolete::eclipseGridFileName );
    m_eclipseGridFileName.uiCapability()->setUiReadOnly( true );
    m_eclipseGridFileName.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitField( &m_includeRestartFiles, "IncludeRestartFiles", false, "Include Restart Files" );
    m_includeRestartFiles.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridSummaryCase_obsolete::~RimGridSummaryCase_obsolete()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString summaryHeaderFilenameFromEclipseCase( RimEclipseCase* eclCase )
{
    if ( !eclCase ) return QString();

    QFileInfo gridFileInfo( eclCase->gridFileName() );

    QString possibleSumHeaderFileName = gridFileInfo.path() + "/" + gridFileInfo.completeBaseName() + ".SMSPEC";

    return possibleSumHeaderFileName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString caseNameFromEclipseCase( RimEclipseCase* eclCase )
{
    if ( !eclCase ) return QString();

    return eclCase->caseUserDescription();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridSummaryCase_obsolete::setAssociatedEclipseCase( RimEclipseCase* eclipseCase )
{
    m_eclipseCase           = eclipseCase;
    m_summaryHeaderFilename = summaryHeaderFilenameFromEclipseCase( eclipseCase );
    m_cachedCaseName        = caseNameFromEclipseCase( eclipseCase );

    this->updateAutoShortName();
    this->updateTreeItemName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimGridSummaryCase_obsolete::associatedEclipseCase()
{
    if ( !m_eclipseCase() )
    {
        // Find a possible associated eclipse case

        RimProject* project;
        firstAncestorOrThisOfTypeAsserted( project );
        std::vector<RimCase*> allCases;
        project->allCases( allCases );
        for ( RimCase* someCase : allCases )
        {
            auto eclCase = dynamic_cast<RimEclipseCase*>( someCase );
            if ( eclCase )
            {
                QString sumHeaderFileName = summaryHeaderFilenameFromEclipseCase( eclCase );
                if ( sumHeaderFileName == m_summaryHeaderFilename().path() )
                {
                    m_eclipseCase = eclCase;
                    this->updateAutoShortName();
                    this->updateTreeItemName();

                    break;
                }
            }
        }
    }

    return m_eclipseCase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridSummaryCase_obsolete::summaryHeaderFilename() const
{
    if ( m_eclipseCase() )
    {
        auto candidate = summaryHeaderFilenameFromEclipseCase( m_eclipseCase );
        if ( QFileInfo::exists( candidate ) ) return candidate;
    }

    return m_summaryHeaderFilename().path();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridSummaryCase_obsolete::caseName() const
{
    if ( m_eclipseCase() ) m_cachedCaseName = caseNameFromEclipseCase( m_eclipseCase() );

    return m_cachedCaseName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridSummaryCase_obsolete::eclipseGridFileName() const
{
    if ( !m_eclipseCase() ) return QString();

    return m_eclipseCase()->gridFileName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridSummaryCase_obsolete::createSummaryReaderInterface()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RimGridSummaryCase_obsolete::summaryReader()
{
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridSummaryCase_obsolete::setIncludeRestartFiles( bool includeRestartFiles )
{
    m_includeRestartFiles = includeRestartFiles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridSummaryCase_obsolete::convertGridCasesToSummaryFileCases( RimProject* project )
{
    if ( !project ) return;

    auto summaryCaseMainCollection = RiaSummaryTools::summaryCaseMainCollection();
    if ( !summaryCaseMainCollection ) return;

    std::vector<RimGridSummaryCase_obsolete*> gridCases;

    auto summaryCases = project->allSummaryCases();
    for ( auto sumCase : summaryCases )
    {
        if ( auto gridCase = dynamic_cast<RimGridSummaryCase_obsolete*>( sumCase ) ) gridCases.push_back( gridCase );
    }

    for ( RimGridSummaryCase_obsolete* gridCase : gridCases )
    {
        RimFileSummaryCase* fileSummaryCase = createFileSummaryCaseCopy( *gridCase );
        summaryCaseMainCollection->addCase( fileSummaryCase );

        std::vector<caf::PdmFieldHandle*> referringFields;
        gridCase->referringPtrFields( referringFields );

        for ( caf::PdmFieldHandle* field : referringFields )
        {
            auto ptrField = dynamic_cast<caf::PdmPtrField<RimSummaryCase*>*>( field );
            if ( ptrField ) ptrField->setValue( fileSummaryCase );
        }
    }

    for ( RimGridSummaryCase_obsolete* gridCase : gridCases )
    {
        summaryCaseMainCollection->removeCase( gridCase );

        delete gridCase;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFileSummaryCase* RimGridSummaryCase_obsolete::createFileSummaryCaseCopy( const RimGridSummaryCase_obsolete& source )
{
    auto fileSummaryCase = new RimFileSummaryCase();
    fileSummaryCase->copyFrom( source );
    fileSummaryCase->setIncludeRestartFiles( source.m_includeRestartFiles() );
    return fileSummaryCase;
}
