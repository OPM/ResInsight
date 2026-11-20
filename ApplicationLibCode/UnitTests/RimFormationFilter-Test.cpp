/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "Formations/RimFormationNames.h"
#include "Formations/RimFormationTools.h"
#include "RimDataFilterCollection.h"
#include "RimEclipseResultCase.h"
#include "RimFormationFilter.h"
#include "RimReservoirGridEnsemble.h"

#include "cafPdmDefaultObjectFactory.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTextStream>

#include <memory>

namespace
{
//--------------------------------------------------------------------------------------------------
/// Formation filter with directly assigned formation categories, as computed from the case data by
/// RimEclipsePropertyFilter::computeResultValueRange
//--------------------------------------------------------------------------------------------------
class TestFormationFilter : public RimFormationFilter
{
public:
    void             setFormations( const std::vector<QString>& names ) { setCategoryNames( names ); }
    std::vector<int> selectedValues() const { return m_selectedCategoryValues(); }
};

//--------------------------------------------------------------------------------------------------
/// Write a .lyr file and read it into a RimFormationNames
//--------------------------------------------------------------------------------------------------
std::unique_ptr<RimFormationNames> createFormationNames( const QTemporaryDir& dir, const QString& fileName, const QString& lyrContent )
{
    const QString filePath = QDir( dir.path() ).absoluteFilePath( fileName );

    QFile file( filePath );
    EXPECT_TRUE( file.open( QIODevice::WriteOnly | QIODevice::Text ) );
    QTextStream stream( &file );
    stream << lyrContent;
    file.close();

    auto formationNames = std::make_unique<RimFormationNames>();
    formationNames->setFileName( filePath );
    EXPECT_TRUE( formationNames->readFormationNamesFile().has_value() );

    return formationNames;
}

const QString referenceLyr   = "'Top'    1 - 2\n'Middle' 3 - 5\n'Bottom' 6 - 8\n";
const QString shiftedKLyr    = "'Top'    1 - 3\n'Middle' 4 - 4\n'Bottom' 5 - 9\n";
const QString reorderedLyr   = "'Middle' 1 - 2\n'Top'    3 - 5\n'Bottom' 6 - 8\n";
const QString extraNameLyr   = "'Top'    1 - 2\n'Middle' 3 - 5\n'Bottom' 6 - 8\n'Base' 9 - 10\n";
const QString missingNameLyr = "'Top'    1 - 2\n'Bottom' 3 - 8\n";
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimFormationTools, formationIndicesFromNames )
{
    const std::vector<QString> available = { "Top", "Middle", "Bottom" };

    auto indices = RimFormationTools::formationIndicesFromNames( available, { "Bottom", "Top" } );
    ASSERT_TRUE( indices.has_value() );
    EXPECT_EQ( ( std::vector<int>{ 0, 2 } ), indices.value() );

    auto all = RimFormationTools::formationIndicesFromNames( available, {} );
    ASSERT_TRUE( all.has_value() );
    EXPECT_EQ( ( std::vector<int>{ 0, 1, 2 } ), all.value() );
}

//--------------------------------------------------------------------------------------------------
/// A formation split over several K ranges occurs more than once, and all occurrences are selected
//--------------------------------------------------------------------------------------------------
TEST( RimFormationTools, formationIndicesFromNamesSelectsAllDuplicates )
{
    const std::vector<QString> available = { "Top", "Middle", "Top", "Bottom" };

    auto indices = RimFormationTools::formationIndicesFromNames( available, { "Top" } );
    ASSERT_TRUE( indices.has_value() );
    EXPECT_EQ( ( std::vector<int>{ 0, 2 } ), indices.value() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimFormationTools, formationIndicesFromUnknownNameIsError )
{
    const std::vector<QString> available = { "Top", "Middle", "Bottom" };

    auto indices = RimFormationTools::formationIndicesFromNames( available, { "Top", "Unknown" } );
    ASSERT_FALSE( indices.has_value() );
    EXPECT_TRUE( indices.error().contains( "Unknown" ) );
    EXPECT_TRUE( indices.error().contains( "Top, Middle, Bottom" ) );

    auto noFormations = RimFormationTools::formationIndicesFromNames( {}, { "Top" } );
    ASSERT_FALSE( noFormations.has_value() );
    EXPECT_TRUE( noFormations.error().contains( "none" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimFormationTools, formationNamesFromIndices )
{
    const std::vector<QString> available = { "Top", "Middle", "Top", "Bottom" };

    EXPECT_EQ( ( std::vector<QString>{ "Top", "Bottom" } ), RimFormationTools::formationNamesFromIndices( available, { 3, 2, 0 } ) );
    EXPECT_EQ( ( std::vector<QString>{ "Middle" } ), RimFormationTools::formationNamesFromIndices( available, { 1, 17, -1 } ) );
    EXPECT_TRUE( RimFormationTools::formationNamesFromIndices( available, {} ).empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimFormationTools, unionOfFormationNames )
{
    const std::vector<QString> names =
        RimFormationTools::unionOfFormationNames( { { "Top", "Middle", "Bottom" }, { "Middle", "Base", "Top" }, {}, { "Cap", "Base" } } );

    EXPECT_EQ( ( std::vector<QString>{ "Top", "Middle", "Bottom", "Base", "Cap" } ), names );
}

//--------------------------------------------------------------------------------------------------
/// Only the ordered names are compared, not the K ranges
//--------------------------------------------------------------------------------------------------
TEST( RimFormationTools, casesWithDifferentFormations )
{
    QTemporaryDir dir;
    ASSERT_TRUE( dir.isValid() );

    auto reference   = createFormationNames( dir, "reference.lyr", referenceLyr );
    auto shiftedK    = createFormationNames( dir, "shifted.lyr", shiftedKLyr );
    auto reordered   = createFormationNames( dir, "reordered.lyr", reorderedLyr );
    auto extraName   = createFormationNames( dir, "extra.lyr", extraNameLyr );
    auto missingName = createFormationNames( dir, "missing.lyr", missingNameLyr );

    std::vector<std::unique_ptr<RimEclipseResultCase>> cases;
    for ( RimFormationNames* formationNames :
          { reference.get(), shiftedK.get(), reordered.get(), extraName.get(), missingName.get(), (RimFormationNames*)nullptr } )
    {
        auto eclipseCase = std::make_unique<RimEclipseResultCase>();
        eclipseCase->setFormationNames( formationNames );
        cases.push_back( std::move( eclipseCase ) );
    }

    std::vector<RimEclipseCase*> casePointers;
    for ( const auto& eclipseCase : cases )
    {
        casePointers.push_back( eclipseCase.get() );
    }

    EXPECT_EQ( ( std::vector<QString>{ "Top", "Middle", "Bottom" } ), RimFormationTools::formationNames( cases[1].get() ) );

    const std::vector<RimEclipseCase*> different = RimFormationTools::casesWithDifferentFormations( cases[0].get(), casePointers );

    const std::vector<RimEclipseCase*> expected = { cases[2].get(), cases[3].get(), cases[4].get(), cases[5].get() };
    EXPECT_EQ( expected, different );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimFormationFilter, setSelectedFormationNames )
{
    TestFormationFilter filter;
    filter.setFormations( { "Top", "Middle", "Top", "Bottom" } );

    EXPECT_EQ( ( std::vector<QString>{ "Top", "Middle", "Top", "Bottom" } ), filter.availableFormationNames() );

    ASSERT_TRUE( filter.setSelectedFormationNames( { "Top" } ).has_value() );
    EXPECT_EQ( ( std::vector<int>{ 0, 2 } ), filter.selectedValues() );
    EXPECT_EQ( ( std::vector<QString>{ "Top" } ), filter.selectedFormationNames() );

    ASSERT_TRUE( filter.setSelectedFormationNames( { "Bottom", "Middle" } ).has_value() );
    EXPECT_EQ( ( std::vector<int>{ 1, 3 } ), filter.selectedValues() );
    EXPECT_EQ( ( std::vector<QString>{ "Middle", "Bottom" } ), filter.selectedFormationNames() );

    ASSERT_TRUE( filter.setSelectedFormationNames( {} ).has_value() );
    EXPECT_EQ( ( std::vector<int>{ 0, 1, 2, 3 } ), filter.selectedValues() );
}

//--------------------------------------------------------------------------------------------------
/// An unknown name is an error, and the previous selection is kept
//--------------------------------------------------------------------------------------------------
TEST( RimFormationFilter, setSelectedUnknownFormationNameKeepsSelection )
{
    TestFormationFilter filter;
    filter.setFormations( { "Top", "Middle", "Bottom" } );

    ASSERT_TRUE( filter.setSelectedFormationNames( { "Middle" } ).has_value() );

    auto result = filter.setSelectedFormationNames( { "Middle", "Unknown" } );
    ASSERT_FALSE( result.has_value() );
    EXPECT_TRUE( result.error().contains( "Unknown" ) );
    EXPECT_EQ( ( std::vector<int>{ 1 } ), filter.selectedValues() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimFormationFilter, setSelectedFormationNamesWithoutFormationsIsError )
{
    RimFormationFilter filter;
    EXPECT_FALSE( filter.setSelectedFormationNames( { "Top" } ).has_value() );
}

//--------------------------------------------------------------------------------------------------
/// Realizations of the ensemble are compared against the main case, which is the first realization
//--------------------------------------------------------------------------------------------------
TEST( RimFormationFilter, realizationsWithMismatchedFormations )
{
    QTemporaryDir dir;
    ASSERT_TRUE( dir.isValid() );

    auto reference = createFormationNames( dir, "reference.lyr", referenceLyr );
    auto shiftedK  = createFormationNames( dir, "shifted.lyr", shiftedKLyr );
    auto reordered = createFormationNames( dir, "reordered.lyr", reorderedLyr );

    auto ensemble = std::make_unique<RimReservoirGridEnsemble>();

    std::vector<RimEclipseCase*> realizations;
    for ( RimFormationNames* formationNames : { reference.get(), shiftedK.get(), reordered.get() } )
    {
        auto* eclipseCase = new RimEclipseResultCase();
        eclipseCase->setFormationNames( formationNames );
        ensemble->addCase( eclipseCase );
        realizations.push_back( eclipseCase );
    }

    auto* filter = new RimFormationFilter();
    ensemble->dataFilterCollection()->addFilter( filter );

    EXPECT_EQ( ensemble.get(), filter->parentEnsemble() );
    EXPECT_EQ( ( std::vector<RimEclipseCase*>{ realizations[2] } ), filter->realizationsWithMismatchedFormations() );

    // A filter outside an ensemble has no realizations
    RimFormationFilter standaloneFilter;
    EXPECT_TRUE( standaloneFilter.realizationsWithMismatchedFormations().empty() );
}

//--------------------------------------------------------------------------------------------------
/// Realizations without formations of their own use the shared formations of the ensemble
//--------------------------------------------------------------------------------------------------
TEST( RimFormationFilter, realizationsWithoutOwnFormationsUseEnsembleFormations )
{
    auto ensemble = std::make_unique<RimReservoirGridEnsemble>();
    ensemble->addCase( new RimEclipseResultCase() );
    ensemble->addCase( new RimEclipseResultCase() );

    auto* filter = new RimFormationFilter();
    ensemble->dataFilterCollection()->addFilter( filter );

    EXPECT_TRUE( filter->realizationsWithMismatchedFormations().empty() );
}

//--------------------------------------------------------------------------------------------------
/// The selection is stored as formation indices
//--------------------------------------------------------------------------------------------------
TEST( RimFormationFilter, pdmRoundTripPreservesSelection )
{
    QString xml;
    {
        RimDataFilterCollection original;

        auto* filter = new TestFormationFilter();
        filter->setFormations( { "Top", "Middle", "Top", "Bottom" } );
        ASSERT_TRUE( filter->setSelectedFormationNames( { "Top" } ).has_value() );
        original.addFilter( filter );

        xml = original.writeObjectToXmlString();
    }

    RimDataFilterCollection restored;
    restored.readObjectFromXmlString( xml, caf::PdmDefaultObjectFactory::instance() );
    restored.resolveReferencesRecursively();

    ASSERT_EQ( size_t{ 1 }, restored.count() );
    auto* filter = dynamic_cast<RimFormationFilter*>( restored.filters().front() );
    ASSERT_NE( nullptr, filter );
    EXPECT_EQ( ( std::vector<int>{ 0, 2 } ), filter->selectedCategoryValues() );
}
