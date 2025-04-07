
#include <string>
#include "gtest/gtest.h"


#include "opm/input/eclipse/Parser/ParseContext.hpp"
#include "opm/input/eclipse/Schedule/VFPInjTable.hpp"
#include "opm/input/eclipse/Schedule/VFPProdTable.hpp"
#include "opm/input/eclipse/Parser/Parser.hpp"
#include "opm/input/eclipse/Parser/InputErrorAction.hpp"
#include "opm/input/eclipse/Deck/Deck.hpp"

#include <opm/input/eclipse/Parser/ParserKeywords/V.hpp>
#include "opm/input/eclipse/Parser/ParserKeywords/W.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/I.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/S.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/P.hpp"

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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(OpmParserTest, ReadAndParseWSEGLINK)
{
    Parser parser(false);

    const Opm::ParserKeywords::WSEGLINK kw1;
    const Opm::ParserKeywords::INCLUDE  kw2;
    const Opm::ParserKeywords::PATHS    kw3;

    parser.addParserKeyword(kw1);
    parser.addParserKeyword(kw2);
    parser.addParserKeyword(kw3);

    std::string testFilePath = std::string(TEST_DATA_DIR) + "/test_wseglink.DATA";
    
    Opm::ParseContext parseContext(Opm::InputErrorAction::WARN);
    auto              deck = parser.parseFile(testFilePath, parseContext);

    std::string myKeyword = "WSEGLINK";
    auto keywordList = deck.getKeywordList(myKeyword);
    for (auto kw : keywordList)
    {
        for (size_t i = 0; i < kw->size(); i++)
        {
            auto deckRecord = kw->getRecord(i);
            
            std::string wellName;
            int segment1 = -1;
            int segment2 = -1;

            {
                auto itemName = ::Opm::ParserKeywords::WSEGLINK::WELL::itemName;
                if (deckRecord.hasItem(itemName) && deckRecord.getItem(itemName).hasValue(0))
                {
                    wellName = deckRecord.getItem(itemName).getTrimmedString(0);
                }
            }
            {
                auto itemName = ::Opm::ParserKeywords::WSEGLINK::SEGMENT1::itemName;
                if (deckRecord.hasItem(itemName) && deckRecord.getItem(itemName).hasValue(0))
                {
                    segment1 = deckRecord.getItem(itemName).get<int>(0);
                }
            }
            {
                auto itemName = ::Opm::ParserKeywords::WSEGLINK::SEGMENT2::itemName;
                if (deckRecord.hasItem(itemName) && deckRecord.getItem(itemName).hasValue(0))
                {
                    segment2 = deckRecord.getItem(itemName).get<int>(0);
                }
            }
                
            std::cout << wellName << " " << segment1 << " " << segment2 << std::endl;
        }
    }
}