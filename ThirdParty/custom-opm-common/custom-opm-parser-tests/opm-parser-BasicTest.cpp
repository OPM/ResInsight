
#include <string>
#include "gtest/gtest.h"

#include "opm/parser/eclipse/Parser/Parser.hpp"
#include "opm/parser/eclipse/EclipseState/Schedule/VFPInjTable.hpp"
#include "opm/parser/eclipse/EclipseState/Schedule/VFPProdTable.hpp"

#include "OpmTestDataDirectory.h"

using namespace Opm;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(OpmParserTest, ReadFromFile)
{
    ParseContext parseContext;

    {
        Parser parser;

        std::stringstream ss;
        ss << TEST_DATA_DIR << "/B1BH.Ecl";
        std::string testFile = ss.str();

        auto deck = parser.parseFile(testFile);

        std::string myKeyword = "VFPPROD";
        auto keywordList = deck.getKeywordList(myKeyword);

        UnitSystem unitSystem;

    
        for (auto kw : keywordList)
        {
            auto name = kw->name();
    
            VFPProdTable table(*kw, unitSystem);
            std::cout << table.getDatumDepth() << std::endl;
        }
    }
    {
        Parser parser;

        std::stringstream ss;
        ss << TEST_DATA_DIR << "/C1H.Ecl";
        std::string testFile = ss.str();

        auto deck = parser.parseFile(testFile);

        std::string myKeyword = "VFPINJ";
        auto keywordList = deck.getKeywordList(myKeyword);

        UnitSystem unitSystem;
    
        for (auto kw : keywordList)
        {
            auto name = kw->name();
    
            VFPInjTable table(*kw, unitSystem);
            std::cout << table.getDatumDepth() << std::endl;
        }
    }


}



    