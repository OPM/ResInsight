/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"

#include "Formations/RimFormationNames.h"
#include "RimEclipseResultCase.h"

#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigReservoirBuilder.h"

#include <QDir>
#include <QFile>
#include <QStringList>

#include <memory>
#include <string>

namespace
{
struct MockCase
{
    std::unique_ptr<RimEclipseResultCase> resultCase;
    cvf::ref<RigEclipseCaseData>          eclipseCase;
};

//--------------------------------------------------------------------------------------------------
/// Build a regular ni x nj x nk box grid in memory (no file, no view)
//--------------------------------------------------------------------------------------------------
MockCase buildBoxGridCase( int ni, int nj, int nk )
{
    RigReservoirBuilder builder;
    builder.setIJKCount( cvf::Vec3st( ni, nj, nk ) );
    builder.setWorldCoordinates( cvf::Vec3d( 0.0, 0.0, 0.0 ), cvf::Vec3d( ni, nj, -nk ) );

    MockCase mockCase;
    mockCase.resultCase.reset( new RimEclipseResultCase );
    mockCase.eclipseCase = new RigEclipseCaseData( mockCase.resultCase.get() );

    builder.createGridsAndCells( mockCase.eclipseCase.p() );
    mockCase.eclipseCase->mainGrid()->computeCachedData();

    mockCase.resultCase->setReservoirData( mockCase.eclipseCase.p() );

    return mockCase;
}

std::unique_ptr<RimFormationNames> readNorneFormationNames()
{
    QDir    baseFolder( TEST_DATA_DIR );
    QString filePath = baseFolder.absoluteFilePath( "RifFormationNamesReader/Norne_ATW2013.lyr" );
    EXPECT_TRUE( QFile::exists( filePath ) );

    auto formationNames = std::make_unique<RimFormationNames>();
    formationNames->setFileName( filePath );

    QString errorMessage;
    formationNames->readFormationNamesFile( &errorMessage );
    EXPECT_TRUE( errorMessage.isEmpty() );

    return formationNames;
}

std::string joinedFormationNames( const RigEclipseCaseData* eclipseCase )
{
    QStringList names;
    for ( const QString& name : eclipseCase->formationNames() )
    {
        names.push_back( name );
    }

    return names.join( "|" ).toStdString();
}
} // namespace

//--------------------------------------------------------------------------------------------------
/// The case data must keep the formation names alive, as reloading or deleting the owning
/// RimFormationNames would otherwise leave a dangling pointer behind.
//--------------------------------------------------------------------------------------------------
TEST( RimFormationNamesTest, CaseDataKeepsFormationNamesAliveAfterReloadAndDelete )
{
    MockCase mockCase       = buildBoxGridCase( 2, 2, 3 );
    auto     formationNames = readNorneFormationNames();

    mockCase.eclipseCase->setActiveFormationNames( formationNames->formationNamesData() );

    const std::string namesBeforeReload = joinedFormationNames( mockCase.eclipseCase.p() );
    EXPECT_FALSE( namesBeforeReload.empty() );

    // Reload replaces the data owned by RimFormationNames
    QString errorMessage;
    formationNames->readFormationNamesFile( &errorMessage );
    EXPECT_TRUE( errorMessage.isEmpty() );

    EXPECT_EQ( namesBeforeReload, joinedFormationNames( mockCase.eclipseCase.p() ) );

    // Deleting the owner must not invalidate the data referenced by the case data
    formationNames.reset();

    EXPECT_EQ( namesBeforeReload, joinedFormationNames( mockCase.eclipseCase.p() ) );
}
