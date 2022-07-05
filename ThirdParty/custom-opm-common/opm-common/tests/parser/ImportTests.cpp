/*
  Copyright 2021 Equinor ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
 */

#define BOOST_TEST_MODULE ImportTests
#include <boost/test/unit_test.hpp>

#include <opm/io/eclipse/EclOutput.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/ImportContainer.hpp>
#include <opm/input/eclipse/Units/UnitSystem.hpp>
#include <tests/WorkArea.hpp>
#include <iostream>
#include <filesystem>
#include <fstream>

#include <opm/input/eclipse/Parser/ParserKeywords/I.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/M.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/P.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/Z.hpp>

using namespace Opm;
namespace fs = std::filesystem;

BOOST_AUTO_TEST_CASE(CreateImportContainer) {
    WorkArea work;
    auto unit_system = UnitSystem::newMETRIC();
    Parser parser;
    BOOST_CHECK_THROW(ImportContainer(parser, unit_system, "/no/such/file", false, 0), std::exception);
    {
        EclIO::EclOutput output {"FILE_NAME", false};
    }

    ImportContainer container1(parser, unit_system, "FILE_NAME", false, 0);
    Deck deck;

    for (auto kw : container1)
        deck.addKeyword(std::move(kw));

    BOOST_CHECK_EQUAL(deck.size(), 0);



    {
        EclIO::EclOutput output {"FILE_NAME", false};
        output.write<double>("PORO", {0, 1, 2, 3, 4});
        output.write<float>("PERMX", {10, 20, 30, 40});
        output.write<int>("FIPNUM", {100, 200, 300, 400});
        output.write<int>("UNKNOWN", {100, 200, 300, 400});
        output.write<float>("MAPAXES", {10, 20, 30, 40, 50, 60});
    }

    ImportContainer container2(parser, unit_system, "FILE_NAME", false, 0);
    for (auto kw : container2)
        deck.addKeyword(std::move(kw));

    BOOST_CHECK_EQUAL(deck.size(), 4);
    BOOST_CHECK( deck.hasKeyword<ParserKeywords::MAPAXES>());
}



BOOST_AUTO_TEST_CASE(ImportDeck) {
    const std::string deck_string = R"(
RUNSPEC

PATHS
'IMPPATH' 'import' /
/

DIMENS
   10 10 10 /

GRID

IMPORT
   'import/GRID' /

IMPORT
   '$IMPPATH/PROPS' /
)";

    WorkArea work;
    auto unit_system = UnitSystem::newMETRIC();
    const std::size_t nx = 10;
    const std::size_t ny = 10;
    const std::size_t nz = 10;

    EclipseGrid grid(nx,ny,nz);
    fs::create_directory("import");
    fs::create_directory("cwd");
    grid.save("import/GRID", false, {}, unit_system);
    {
        EclIO::EclOutput output {"import/PROPS", false};

        std::vector<double> poro{nx*ny*nz, 0.25};
        std::vector<float>  perm{nx*ny*nz, 100};
        output.write<double>("PORO", poro);
        output.write<float>("PERMX", perm);
        output.write<float>("PERMY", perm);
        output.write<float>("PERMZ", perm);
    }

    {
        FILE * stream = fopen("DECK.DATA", "w");
        fprintf(stream, "%s", deck_string.c_str());
        fclose(stream);
    }

    fs::current_path( fs::path("cwd") );
    Parser parser;

    auto deck = parser.parseFile( "../DECK.DATA" );

    BOOST_CHECK( deck.hasKeyword<ParserKeywords::ZCORN>() );
    BOOST_CHECK( deck.hasKeyword<ParserKeywords::PERMX>() );
    BOOST_CHECK( deck.hasKeyword<ParserKeywords::PORO>() );
    BOOST_CHECK( !deck.hasKeyword<ParserKeywords::IMPORT>() );

}
