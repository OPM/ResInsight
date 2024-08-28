/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
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

#include "RifReaderInterface.h"

#include "RiaPreferencesGrid.h"

#include "RifEclipseInputFileTools.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderInterface::RifReaderInterface()
{
    RiaPreferencesGrid* prefs = RiaPreferencesGrid::current();
    m_readerSettings          = prefs->readerSettings();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderInterface::isFaultImportEnabled() const
{
    return m_readerSettings.importFaults;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderInterface::isImportOfCompleteMswDataEnabled() const
{
    return m_readerSettings.importAdvancedMswData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderInterface::isNNCsEnabled() const
{
    return m_readerSettings.importNNCs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderInterface::loadWellDataEnabled() const
{
    return !m_readerSettings.skipWellData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderInterface::includeInactiveCellsInFaultGeometry() const
{
    return m_readerSettings.includeInactiveCellsInFaultGeometry;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString RifReaderInterface::faultIncludeFileAbsolutePathPrefix() const
{
    return m_readerSettings.includeFileAbsolutePathPrefix;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderInterface::onlyLoadActiveCells() const
{
    return m_readerSettings.onlyLoadActiveCells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderInterface::setTimeStepFilter( const std::vector<size_t>& fileTimeStepIndices )
{
    m_fileTimeStepIndices = fileTimeStepIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RiaDefines::PhaseType> RifReaderInterface::availablePhases() const
{
    return std::set<RiaDefines::PhaseType>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderInterface::isTimeStepIncludedByFilter( size_t timeStepIndex ) const
{
    if ( m_fileTimeStepIndices.empty() ) return true;

    for ( auto i : m_fileTimeStepIndices )
    {
        if ( i == timeStepIndex )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RifReaderInterface::timeStepIndexOnFile( size_t timeStepIndex ) const
{
    if ( timeStepIndex < m_fileTimeStepIndices.size() )
    {
        return m_fileTimeStepIndices[timeStepIndex];
    }

    return timeStepIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderInterface::setReaderSettings( RifReaderSettings readerSettings )
{
    m_readerSettings = readerSettings;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderInterface::importFaults( const QStringList& fileSet, cvf::Collection<RigFault>* faults )
{
    if ( !filenamesWithFaults().empty() )
    {
        for ( size_t i = 0; i < filenamesWithFaults().size(); i++ )
        {
            QString faultFilename = filenamesWithFaults()[i];

            RifEclipseInputFileTools::parseAndReadFaults( faultFilename, faults );
        }
    }
    else
    {
        foreach ( QString fname, fileSet )
        {
            if ( fname.endsWith( ".DATA" ) )
            {
                std::vector<QString> filenamesWithFaults;
                RifEclipseInputFileTools::readFaultsInGridSection( fname, faults, &filenamesWithFaults, faultIncludeFileAbsolutePathPrefix() );

                std::sort( filenamesWithFaults.begin(), filenamesWithFaults.end() );
                std::vector<QString>::iterator last = std::unique( filenamesWithFaults.begin(), filenamesWithFaults.end() );
                filenamesWithFaults.erase( last, filenamesWithFaults.end() );

                setFilenamesWithFaults( filenamesWithFaults );
            }
        }
    }
}
