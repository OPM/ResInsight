/*
  Copyright 2014 by Andreas Lauser

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


#define BOOST_TEST_MODULE ParserTests
#include <boost/filesystem/path.hpp>
#include <boost/test/unit_test.hpp>

#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>


BOOST_AUTO_TEST_CASE(Verify_find_includes_Data_file_is_a_symlink) {
    boost::filesystem::path inputFilePath("testdata/parser/includeSymlinkTestdata/symlink1/case_symlink.data");
    Opm::ParserPtr parser(new Opm::Parser());
    Opm::DeckConstPtr deck = parser->parseFile(inputFilePath.string() , Opm::ParseContext());

    BOOST_CHECK_EQUAL(true , deck->hasKeyword("OIL"));
    BOOST_CHECK_EQUAL(false , deck->hasKeyword("WATER"));
}


BOOST_AUTO_TEST_CASE(Verify_find_includes_Data_file_has_include_that_is_a_symlink) {
    boost::filesystem::path inputFilePath("testdata/parser/includeSymlinkTestdata/symlink2/caseWithIncludedSymlink.data");
    Opm::ParserPtr parser(new Opm::Parser());
    Opm::DeckConstPtr deck = parser->parseFile(inputFilePath.string() , Opm::ParseContext());

    BOOST_CHECK_EQUAL(true , deck->hasKeyword("OIL"));
    BOOST_CHECK_EQUAL(false , deck->hasKeyword("WATER"));
}


BOOST_AUTO_TEST_CASE(Verify_find_includes_Data_file_has_include_file_that_again_includes_a_symlink) {
    boost::filesystem::path inputFilePath("testdata/parser/includeSymlinkTestdata/symlink3/case.data");
    Opm::ParserPtr parser(new Opm::Parser());
    Opm::DeckConstPtr deck = parser->parseFile(inputFilePath.string() , Opm::ParseContext());

    BOOST_CHECK_EQUAL(true , deck->hasKeyword("OIL"));
    BOOST_CHECK_EQUAL(false , deck->hasKeyword("WATER"));
}



BOOST_AUTO_TEST_CASE(ParserKeyword_includeValid) {
    boost::filesystem::path inputFilePath("testdata/parser/includeValid.data");

    Opm::ParserPtr parser(new Opm::Parser());
    Opm::DeckConstPtr deck = parser->parseFile(inputFilePath.string() , Opm::ParseContext());

    BOOST_CHECK_EQUAL(true , deck->hasKeyword("OIL"));
    BOOST_CHECK_EQUAL(false , deck->hasKeyword("WATER"));
}


BOOST_AUTO_TEST_CASE(ParserKeyword_includeInvalid) {
    boost::filesystem::path inputFilePath("testdata/parser/includeInvalid.data");

    Opm::ParserPtr parser(new Opm::Parser());
    BOOST_CHECK_THROW(parser->parseFile(inputFilePath.string() , Opm::ParseContext()), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(ParserKeyword_includeWrongCase) {
    boost::filesystem::path inputFile1Path("testdata/parser/includeWrongCase1.data");
    boost::filesystem::path inputFile2Path("testdata/parser/includeWrongCase2.data");
    boost::filesystem::path inputFile3Path("testdata/parser/includeWrongCase3.data");

    Opm::ParserPtr parser(new Opm::Parser());

#if HAVE_CASE_SENSITIVE_FILESYSTEM
    // so far, we expect the files which are included to exhibit
    // exactly the same spelling as their names on disk. Eclipse seems
    // to be a bit more relaxed when it comes to this, so we might
    // have to change the current behavior one not-so-fine day...
    BOOST_CHECK_THROW(parser->parseFile(inputFile1Path.string(), Opm::ParseContext()), std::runtime_error);
    BOOST_CHECK_THROW(parser->parseFile(inputFile2Path.string(), Opm::ParseContext()), std::runtime_error);
    BOOST_CHECK_THROW(parser->parseFile(inputFile3Path.string(), Opm::ParseContext()), std::runtime_error);
#else
    // for case-insensitive filesystems, the include statement will
    // always work regardless of how the capitalization of the
    // included files is wrong...
    BOOST_CHECK_EQUAL(true, parser->parseFile(inputFile1Path.string(), Opm::ParseContext())->hasKeyword("OIL"));
    BOOST_CHECK_EQUAL(false, parser->parseFile(inputFile1Path.string(), Opm::ParseContext())->hasKeyword("WATER"));
    BOOST_CHECK_EQUAL(true, parser->parseFile(inputFile2Path.string(), Opm::ParseContext())->hasKeyword("OIL"));
    BOOST_CHECK_EQUAL(false, parser->parseFile(inputFile2Path.string(), Opm::ParseContext())->hasKeyword("WATER"));
    BOOST_CHECK_EQUAL(true, parser->parseFile(inputFile3Path.string(), Opm::ParseContext())->hasKeyword("OIL"));
    BOOST_CHECK_EQUAL(false, parser->parseFile(inputFile3Path.string(), Opm::ParseContext())->hasKeyword("WATER"));
#endif
}

