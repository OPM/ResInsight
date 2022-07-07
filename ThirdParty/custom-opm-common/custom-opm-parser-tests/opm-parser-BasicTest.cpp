
#include <string>
#include "gtest/gtest.h"


#include "opm/input/eclipse/Parser/ParseContext.hpp"
#include "opm/input/eclipse/Parser/ParseContext.hpp"
#include "opm/input/eclipse/Schedule/VFPInjTable.hpp"
#include "opm/input/eclipse/Schedule/VFPProdTable.hpp"
#include "opm/input/eclipse/Parser/Parser.hpp"
#include "opm/input/eclipse/Deck/Deck.hpp"
#include <opm/input/eclipse/Parser/ParserKeywords/V.hpp>

#include "OpmTestDataDirectory.h"

using namespace Opm;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(OpmParserTest, ReadFromFile)
{
    ParseContext parseContext;

    {
        Parser parser(false);
		const ::Opm::ParserKeywords::VFPPROD kw1;

		parser.addParserKeyword(kw1);

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
    
            bool gaslift_opt_active = false;
            VFPProdTable table(*kw, gaslift_opt_active, unitSystem);
            std::cout << table.getDatumDepth() << std::endl;
        }
    }
    {
		Parser parser(false);
		const ::Opm::ParserKeywords::VFPINJ kw1;
		const ::Opm::ParserKeywords::VFPIDIMS kw2;

		parser.addParserKeyword(kw1);
		parser.addParserKeyword(kw2);

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



    