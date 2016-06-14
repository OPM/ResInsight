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

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <opm/json/JsonObject.hpp>
#include <opm/parser/eclipse/Generator/KeywordGenerator.hpp>
#include <opm/parser/eclipse/Generator/KeywordLoader.hpp>
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {

    KeywordGenerator::KeywordGenerator(bool verbose)
        : m_verbose( verbose )
    {
    }




    std::string testHeader() {
        std::string header = "#define BOOST_TEST_MODULE ParserRecordTests\n"
            "#include <boost/filesystem.hpp>\n"
            "#include <boost/test/unit_test.hpp>\n"
            "#include <memory>\n"
            "#include <opm/json/JsonObject.hpp>\n"
            "#include <opm/parser/eclipse/Parser/ParserKeywords.hpp>\n"
            "#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>\n"
            "#include <opm/parser/eclipse/Parser/ParserItem.hpp>\n"
            "#include <opm/parser/eclipse/Parser/ParserIntItem.hpp>\n"
            "#include <opm/parser/eclipse/Parser/ParserStringItem.hpp>\n"
            "#include <opm/parser/eclipse/Parser/ParserDoubleItem.hpp>\n"
            "#include <opm/parser/eclipse/Parser/ParserRecord.hpp>\n"
            "#include <opm/parser/eclipse/Units/UnitSystem.hpp>\n"
            "using namespace Opm;\n"
            "std::shared_ptr<UnitSystem> unitSystem( UnitSystem::newMETRIC() );\n";

        return header;
    }


    std::string KeywordGenerator::sourceHeader() {
        std::string header = "#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>\n"
            "#include <opm/parser/eclipse/Parser/ParserItem.hpp>\n"
            "#include <opm/parser/eclipse/Parser/ParserIntItem.hpp>\n"
            "#include <opm/parser/eclipse/Parser/ParserStringItem.hpp>\n"
            "#include <opm/parser/eclipse/Parser/ParserDoubleItem.hpp>\n"
            "#include <opm/parser/eclipse/Parser/ParserRecord.hpp>\n"
            "#include <opm/parser/eclipse/Parser/Parser.hpp>\n"
            "#include <opm/parser/eclipse/Parser/ParserKeywords.hpp>\n\n\n"
            "namespace Opm {\n"
            "namespace ParserKeywords {\n\n";

        return header;
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
        boost::filesystem::path file(file_name);
        if (!boost::filesystem::is_directory( file.parent_path()))
            boost::filesystem::create_directories( file.parent_path());
    }

    bool KeywordGenerator::updateFile(const std::stringstream& newContent , const std::string& filename) {
        bool update = true;
        {
            // Check if file already contains the newContent.
            std::ifstream inputStream(filename);
            if (inputStream) {
                std::stringstream oldContent;
                oldContent << inputStream.rdbuf();
                if (oldContent.str() == newContent.str()) {
                    update = false;
                }
            }
        }

        if (update) {
            ensurePath(filename);
            std::ofstream outputStream(filename);
            outputStream << newContent.str();
        }

        return update;
    }

    static bool write_file( const std::stringstream& stream, const std::string& file, bool verbose, std::string desc = "source" ) {
        auto update = KeywordGenerator::updateFile( stream, file );
        if( !verbose ) return update;

        if( update )
            std::cout << "Updated " << desc << " file written to: " << file << std::endl;
        else
            std::cout << "No changes to " << desc << " file: " << file << std::endl;

        return update;
    }


    bool KeywordGenerator::updateSource(const KeywordLoader& loader , const std::string& sourceFile, int blocks ) const {
        std::stringstream newSource;

        const int keywords = loader.size();
        const int blocksize = (keywords / blocks) + 1;

        std::vector< std::stringstream > streams( blocks );
        for( unsigned int i = 0; i < streams.size(); ++i )
            streams[ i ] << sourceHeader() << std::endl
                << "void addDefaultKeywords" << i << "(Parser& p) {" << std::endl;

        int bi = 0;
        for( auto iter = loader.keyword_begin(); iter != loader.keyword_end(); ++iter ) {
            auto block = bi++ / blocksize;
            streams[ block ] << "p.addKeyword< ParserKeywords::"
                << iter->second->className() << " >();" << std::endl;
        }

        for( auto& stream : streams ) stream << "}}}" << std::endl;

        for( unsigned int i = 0; i < streams.size(); ++i ) {
            auto srcfile = sourceFile;
            updateFile( streams[i], srcfile.insert( srcfile.size() - 4, std::to_string( i ) ) );
        }

        newSource << sourceHeader();
        for (auto iter = loader.keyword_begin(); iter != loader.keyword_end(); ++iter) {
            std::shared_ptr<ParserKeyword> keyword = (*iter).second;
            newSource << keyword->createCode() << std::endl;
        }

        for( auto i = 0; i < blocks; ++i )
            newSource << "void addDefaultKeywords" << i << "(Parser& p);" << std::endl;

        newSource << "}" << std::endl;

        newSource << "void Parser::addDefaultKeywords() {" << std::endl;
        for( auto i = 0; i < blocks; ++i )
            newSource << "Opm::ParserKeywords::addDefaultKeywords" << i << "(*this);" << std::endl;

        newSource << "}}" << std::endl;

        return write_file( newSource, sourceFile, m_verbose, "source" );
    }

    bool KeywordGenerator::updateHeader(const KeywordLoader& loader, const std::string& headerBuildPath, const std::string& headerFile) const {
        bool update = false;

        std::map< char, std::vector< const ParserKeyword* > > keywords;
        for( auto iter = loader.keyword_begin(); iter != loader.keyword_end(); ++iter )
            keywords[ std::toupper( iter->second->className().at(0) ) ].push_back( iter->second.get() );

        for( const auto& iter : keywords ) {
            std::stringstream stream;

            stream << headerHeader( std::string( 1, std::toupper( iter.first ) ) );
            for( auto& kw : iter.second )
                stream << kw->createDeclaration("   ") << std::endl;

            stream << "}" << std::endl << "}" << std::endl;
            stream << "#endif" << std::endl;

            const auto final_path = headerBuildPath + headerFile + "/" + std::string( 1, iter.first ) + ".hpp";
            if( write_file( stream, final_path, m_verbose, "header" ) )
                update = true;
        }

        std::stringstream stream;
        stream << headerHeader("");
        stream << "}}" << std::endl;

        for( const auto& iter : keywords )
            stream << "#include <"
                << headerFile + "/"
                << std::string( 1, std::toupper( iter.first ) ) + ".hpp>"
                << std::endl;

        stream << "#endif" << std::endl;

        const auto final_path = headerBuildPath + headerFile + ".hpp";
        return write_file( stream, final_path, m_verbose, "header" ) || update;
    }


    std::string KeywordGenerator::startTest(const std::string& keyword_name) {
        return std::string("BOOST_AUTO_TEST_CASE(TEST") + keyword_name + std::string("Keyword) {\n");
    }


    std::string KeywordGenerator::endTest() {
        return "}\n\n";
    }



    bool KeywordGenerator::updateTest(const KeywordLoader& loader , const std::string& testFile) const {
        std::stringstream stream;

        stream << testHeader();
        for (auto iter = loader.keyword_begin(); iter != loader.keyword_end(); ++iter) {
            const std::string& keywordName = (*iter).first;
            std::shared_ptr<ParserKeyword> keyword = (*iter).second;
            stream << startTest(keywordName);
            stream << "    std::string jsonFile = \"" << loader.getJsonFile( keywordName) << "\";" << std::endl;
            stream << "    boost::filesystem::path jsonPath( jsonFile );" << std::endl;
            stream << "    Json::JsonObject jsonConfig( jsonPath );" << std::endl;
            stream << "    ParserKeyword jsonKeyword(jsonConfig);" << std::endl;
            stream << "    ParserKeywords::" << keywordName << " inlineKeyword;" << std::endl;
            stream << "    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));" << std::endl;
            stream << "    if (jsonKeyword.hasDimension()) {" <<std::endl;
            stream << "        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);" << std::endl;
            stream << "        for (size_t i=0; i < parserRecord->size(); i++){ " << std::endl;
            stream << "            ParserItemConstPtr item = parserRecord->get( i );" << std::endl;
            stream << "            for (size_t j=0; j < item->numDimensions(); j++) {" << std::endl;
            stream << "                std::string dimString = item->getDimension(j);" << std::endl;
            stream << "                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));" << std::endl;
            stream << "             }" << std::endl;
            stream << "        }" << std::endl;
            stream << "    }" << std::endl;
            stream << endTest(  );
        }

        return updateFile( stream , testFile );
    }
}


