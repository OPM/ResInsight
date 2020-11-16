/*
  Copyright 2015 Statoil ASA.

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

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cctype>

#include <opm/common/utility/FileSystem.hpp>

#include <opm/json/JsonObject.hpp>
#include <opm/parser/eclipse/Generator/KeywordGenerator.hpp>
#include <opm/parser/eclipse/Generator/KeywordLoader.hpp>
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>


namespace {


const std::string sourceHeader =
    "#include <opm/parser/eclipse/Deck/UDAValue.hpp>\n"
    "#include <opm/parser/eclipse/Parser/ParserItem.hpp>\n"
    "#include <opm/parser/eclipse/Parser/ParserRecord.hpp>\n"
    "#include <opm/parser/eclipse/Parser/Parser.hpp>\n\n\n";
}

namespace Opm {

    KeywordGenerator::KeywordGenerator(bool verbose)
        : m_verbose( verbose )
    {
    }

    std::string KeywordGenerator::headerHeader(const std::string& suffix) {
        std::string header = "#ifndef PARSER_KEYWORDS_" + suffix + "_HPP\n"
            "#define PARSER_KEYWORDS_" + suffix + "_HPP\n"
            "#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>\n"
            "namespace Opm {\n"
            "namespace ParserKeywords {\n\n";

        return header;
    }

    void KeywordGenerator::ensurePath( const std::string& file_name) {
        Opm::filesystem::path file(file_name);
        if (!Opm::filesystem::is_directory( file.parent_path()))
            Opm::filesystem::create_directories( file.parent_path());
    }

    void KeywordGenerator::updateFile(const std::stringstream& newContent , const std::string& filename) {
        ensurePath(filename);
        std::ofstream outputStream(filename);
        outputStream << newContent.str();
    }

    static void write_file( const std::stringstream& stream, const std::string& file, bool verbose, std::string desc = "source" ) {
        KeywordGenerator::updateFile( stream, file );
        if( verbose )
            std::cout << "Updated " << desc << " file written to: " << file << std::endl;
    }

    void KeywordGenerator::updateInitSource(const KeywordLoader& loader , const std::string& sourceFile ) const {
        std::stringstream newSource;
        newSource << "#include <opm/parser/eclipse/Parser/Parser.hpp>" << std::endl;
        for(const auto& kw_pair : loader) {
            const auto& first_char = kw_pair.first;
            newSource << "#include <opm/parser/eclipse/Parser/ParserKeywords/" << first_char << ".hpp>" << std::endl;
        }
        newSource << "namespace Opm {" << std::endl;
        newSource << "namespace ParserKeywords {" << std::endl;
        newSource << "void addDefaultKeywords(Parser& p);"  << std::endl
                  << "void addDefaultKeywords(Parser& p) {" << std::endl;

        for(const auto& kw_pair : loader) {
            const auto& keywords = kw_pair.second;
            for (const auto& kw: keywords)
                newSource << "   p.addKeyword< ParserKeywords::"
                          << kw.className()
                          << " >();" << std::endl;
        }
        newSource << "}" << std::endl;
        newSource << "}" << std::endl;

        newSource << "void Parser::addDefaultKeywords() {\n    ParserKeywords::addDefaultKeywords(*this);\n}" << std::endl;
        newSource << "}" << std::endl;
        write_file( newSource, sourceFile, m_verbose, "init" );
    }

    void KeywordGenerator::updateKeywordSource(const KeywordLoader& loader , const std::string& sourcePath ) const {

        for(const auto& kw_pair : loader) {
            const auto& first_char = kw_pair.first;
            const auto& keywords = kw_pair.second;
            std::stringstream newSource;
            newSource << sourceHeader << std::endl;
            newSource << std::endl << std::endl << "#include <opm/parser/eclipse/Parser/ParserKeywords/" << first_char << ".hpp>" << std::endl;
            newSource << "namespace Opm {" << std::endl;
            newSource << "namespace ParserKeywords {" << std::endl;
            for (const auto& kw: keywords)
                newSource << kw.createCode() << std::endl;
            newSource << "}\n}" << std::endl;
            write_file( newSource, sourcePath + "/" + first_char + ".cpp", m_verbose, "source" );
        }

    }

    void KeywordGenerator::updateHeader(const KeywordLoader& loader, const std::string& headerBuildPath, const std::string& headerPath) const {
        for( const auto& kw_pair : loader) {
            std::stringstream stream;
            const auto& first_char = kw_pair.first;
            const auto& keywords = kw_pair.second;

            stream << headerHeader( std::string( 1, std::toupper( first_char ) ) );
            for( auto& kw : keywords )
                stream << kw.createDeclaration("   ") << std::endl;

            stream << "}" << std::endl << "}" << std::endl;
            stream << "#endif" << std::endl;

            const auto final_path = headerBuildPath + headerPath+ "/" + std::string( 1, first_char ) + ".hpp";
            write_file( stream, final_path, m_verbose, "header" );
        }
    }


    std::string KeywordGenerator::startTest(const std::string& keyword_name) {
        return std::string("BOOST_AUTO_TEST_CASE(TEST") + keyword_name + std::string("Keyword) {\n");
    }


    std::string KeywordGenerator::endTest() {
        return "}\n\n";
    }



    void KeywordGenerator::updateTest(const KeywordLoader& loader , const std::string& testFile) const {
        std::stringstream stream;

        for(const auto& kw_pair : loader) {
            const auto& first_char = kw_pair.first;
            stream << "#include <opm/parser/eclipse/Parser/ParserKeywords/" << first_char << ".hpp>" << std::endl;
        }

        stream << R"(

#define BOOST_TEST_MODULE GeneratedKeywordTest
#include <opm/common/utility/FileSystem.hpp>
#include <boost/test/unit_test.hpp>
#include <memory>
#include <opm/json/JsonObject.hpp>
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
#include <opm/parser/eclipse/Parser/ParserItem.hpp>
#include <opm/parser/eclipse/Parser/ParserRecord.hpp>
#include <opm/parser/eclipse/Units/UnitSystem.hpp>
auto unitSystem =  Opm::UnitSystem::newMETRIC();

namespace Opm {
void test_keyword(const ParserKeyword& inline_keyword, const std::string& json_file) {
    Opm::filesystem::path jsonPath( json_file );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword json_keyword(jsonConfig);
    BOOST_CHECK_EQUAL( json_keyword, inline_keyword);
    if (json_keyword.hasDimension()) {
        const auto& parserRecord = json_keyword.getRecord(0);
        for (size_t i=0; i < parserRecord.size(); i++){
            const auto& item = parserRecord.get( i );
            for (const auto& dim : item.dimensions())
                BOOST_CHECK_NO_THROW( unitSystem.getNewDimension( dim ));
        }
    }
}


)";

        for(const auto& kw_pair : loader) {
            stream << std::endl << "BOOST_AUTO_TEST_CASE(TestKeywords" << kw_pair.first << ") {" << std::endl;
            const auto& keywords = kw_pair.second;
            for (const auto& kw: keywords)
                stream << "    test_keyword( ParserKeywords::" << kw.getName() << "(),\"" << loader.getJsonFile( kw.getName()) << "\");" << std::endl;
            stream << "}" << std::endl;
        }
        stream << "}" << std::endl;
        updateFile( stream , testFile );
    }
}


