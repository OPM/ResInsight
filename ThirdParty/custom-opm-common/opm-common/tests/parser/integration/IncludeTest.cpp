/*
 Copyright 2013 Statoil ASA.

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

#define BOOST_TEST_MODULE ParserIntegrationTests
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include <opm/common/utility/FileSystem.hpp>
#include <iostream>
#include <ostream>
#include <fstream>
#include <iostream>

#include <opm/input/eclipse/Deck/Deck.hpp>

#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Parser/ParserRecord.hpp>

#include <opm/input/eclipse/Parser/ParserEnums.hpp>

using namespace Opm;
using namespace std::filesystem;

static void
createDeckWithInclude(path& datafile, std::string addEndKeyword)
{
    path tmpdir = temp_directory_path();
    path root = tmpdir / unique_path("%%%%-%%%%");
    path absoluteInclude = root / "absolute.include";
    path includePath1 = root / "include";
    path includePath2 = root / "include2";
    path pathIncludeFile = "path.file";

    create_directories(root);
    create_directories(includePath1);
    create_directories(includePath2);

        {
            datafile = root / "TEST.DATA";

            std::ofstream of(datafile.string().c_str());

            of << "PATHS" << std::endl;
            of << "PATH1 '" << includePath1.string() << "' /" << std::endl;
            of << "PATH2 '" << includePath2.string() << "' /" << std::endl;
            of << "/" << std::endl;

            of << "INCLUDE" << std::endl;
            of << "   \'relative.include\' /" << std::endl;

            of << std::endl;

            of << "INCLUDE" << std::endl;
            of << "   \'" << absoluteInclude.string() << "\' /" << std::endl;

            of << std::endl;

            of << "INCLUDE" << std::endl;
            of << "  \'include/nested.include\'   /" << std::endl;

            of << std::endl;



            of << std::endl;

            of << "INCLUDE" << std::endl;
            of << "  \'$PATH1/" << pathIncludeFile.string() << "\'   /" << std::endl;

            of << std::endl;

            of << "INCLUDE" << std::endl;
            of << "  \'$PATH2/" << pathIncludeFile.string() << "\'   /" << std::endl;

            of.close();
        }

        {
            path relativeInclude = root / "relative.include";
            std::ofstream of(relativeInclude.string().c_str());

            of << "START" << std::endl;
            of << "   10 'FEB' 2012 /" << std::endl;
            of.close();
        }

        {
            std::ofstream of(absoluteInclude.string().c_str());

            if (addEndKeyword.length() > 0) {
                of << addEndKeyword << std::endl;
            }
            of << "DIMENS" << std::endl;
            of << "   10 20 30 /" << std::endl;
            of.close();
        }

        {
            path nestedInclude = includePath1 / "nested.include";
            path gridInclude = includePath1 / "grid.include";
            std::ofstream of(nestedInclude.string().c_str());

            of << "INCLUDE" << std::endl;
            of << "   \'$PATH1/grid.include\'  /" << std::endl;
            of.close();

            std::ofstream of2(gridInclude.string().c_str());
            of2 << "GRIDUNIT" << std::endl;
            of2 << "/" << std::endl;
            of2.close();
        }

        {
            path fullPathToPathIncludeFile1 = includePath1 / pathIncludeFile;
            std::ofstream of1(fullPathToPathIncludeFile1.string().c_str());
            of1 << "TITLE" << std::endl;
            of1 << "This is the title /" << std::endl;
            of1.close();

            path fullPathToPathIncludeFile2 = includePath2 / pathIncludeFile;
            std::ofstream of2(fullPathToPathIncludeFile2.string().c_str());
            of2 << "BOX" << std::endl;
            of2 << " 1 2 3 4 5 6 /" << std::endl;
            of2.close();
        }

    std::cout << datafile << std::endl;


}




BOOST_AUTO_TEST_CASE(parse_fileWithWWCTKeyword_deckReturned) {
    path datafile;
    Parser parser;
    createDeckWithInclude (datafile, "");
    auto deck =  parser.parseFile(datafile.string());

    BOOST_CHECK( deck.hasKeyword("START"));
    BOOST_CHECK( deck.hasKeyword("DIMENS"));
    BOOST_CHECK( deck.hasKeyword("GRIDUNIT"));
}

BOOST_AUTO_TEST_CASE(parse_fileWithENDINCKeyword_deckReturned) {
    path datafile;
    Parser parser;
    createDeckWithInclude (datafile, "ENDINC");
    auto deck =  parser.parseFile(datafile.string());

    BOOST_CHECK( deck.hasKeyword("START"));
    BOOST_CHECK( !deck.hasKeyword("DIMENS"));
    BOOST_CHECK( deck.hasKeyword("GRIDUNIT"));
}

BOOST_AUTO_TEST_CASE(parse_fileWithENDKeyword_deckReturned) {
    path datafile;
    Parser parser;
    createDeckWithInclude (datafile, "END");
    auto deck =  parser.parseFile(datafile.string());

    BOOST_CHECK( deck.hasKeyword("START"));
    BOOST_CHECK( !deck.hasKeyword("DIMENS"));
    BOOST_CHECK( !deck.hasKeyword("GRIDUNIT"));
}

BOOST_AUTO_TEST_CASE(parse_fileWithPathsKeyword_IncludeExtendsPath) {
    path datafile;
    Parser parser;
    createDeckWithInclude (datafile, "");
    auto deck =  parser.parseFile(datafile.string());

    BOOST_CHECK( deck.hasKeyword("TITLE"));
    BOOST_CHECK( deck.hasKeyword("BOX"));
}

