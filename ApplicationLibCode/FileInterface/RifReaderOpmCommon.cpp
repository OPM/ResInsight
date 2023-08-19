/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RifReaderOpmCommon.h"
#include "RiaEclipseFileNameTools.h"
#include "RiaLogging.h"
#include "RifEclipseOutputFileTools.h"
#include "RifOpmGridTools.h"
#include "RigEclipseCaseData.h"
#include "opm/input/eclipse/Deck/Deck.hpp"
#include "opm/input/eclipse/EclipseState/Runspec.hpp"
#include "opm/input/eclipse/Parser/Parser.hpp"
#include "opm/io/eclipse/ERst.hpp"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderOpmCommon::RifReaderOpmCommon()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderOpmCommon::~RifReaderOpmCommon()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderOpmCommon::open( const QString& fileName, RigEclipseCaseData* eclipseCase )
{
    QStringList fileSet;
    {
        // auto task = progress.task( "Get set of files" );

        if ( !RifEclipseOutputFileTools::findSiblingFilesWithSameBaseName( fileName, &fileSet ) ) return false;
    }

    try
    {
        m_gridFileName = fileName.toStdString();

        if ( !RifOpmGridTools::importGrid( m_gridFileName, eclipseCase->mainGrid(), eclipseCase ) )
        {
            RiaLogging::error( "Failed to open grid file " + fileName );

            return false;
        }

        buildMetaData();

        return true;
    }
    catch ( std::exception& e )
    {
        auto description = e.what();
        RiaLogging::error( description );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderOpmCommon::staticResult( const QString& result, RiaDefines::PorosityModelType matrixOrFracture, std::vector<double>* values )
{
    throw std::logic_error( "The method or operation is not implemented." );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderOpmCommon::dynamicResult( const QString&                result,
                                        RiaDefines::PorosityModelType matrixOrFracture,
                                        size_t                        stepIndex,
                                        std::vector<double>*          values )
{
    throw std::logic_error( "The method or operation is not implemented." );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderOpmCommon::buildMetaData()
{
    Opm::Deck deck;

    // It is required to create a deck as the input parameter to runspec. The = default() initialization of the runspec keyword does not
    // initialize the object as expected.
    Opm::Runspec runspec( deck );
    Opm::Parser  parser( false );

    // Get set of files
    QStringList fileSet;
    RifEclipseOutputFileTools::findSiblingFilesWithSameBaseName( QString::fromStdString( m_gridFileName ), &fileSet );

    std::string initFileName;
    std::string restartFileName;

    const QString initExt =
        caf::AppEnum<RiaEclipseFileNameTools::EclipseFileType>::text( RiaEclipseFileNameTools::EclipseFileType::ECLIPSE_INIT );

    const QString restartExt =
        caf::AppEnum<RiaEclipseFileNameTools::EclipseFileType>::text( RiaEclipseFileNameTools::EclipseFileType::ECLIPSE_UNRST );

    for ( const auto& s : fileSet )
    {
        if ( s.endsWith( initExt, Qt::CaseInsensitive ) ) initFileName = s.toStdString();
        if ( s.endsWith( restartExt, Qt::CaseInsensitive ) ) restartFileName = s.toStdString();
    }

    if ( !initFileName.empty() )
    {
        m_initFile = std::make_unique<Opm::EclIO::EclFile>( initFileName );
    }
    if ( !restartFileName.empty() )
    {
        m_restartFile = std::make_unique<Opm::EclIO::ERst>( restartFileName );
    }
}
