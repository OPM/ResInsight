#include "gtest/gtest.h"


/*
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include "opm/parser/eclipse/Parser/ParseContext.hpp"
#include "opm/json/JsonObject.hpp"
#include "opm/parser/eclipse/Parser/ParserStringItem.hpp"
#include "opm/parser/eclipse/Parser/ParserRecord.hpp"
*/

/*
#include "custom-opm-common/opm-common/opm/common/OpmLog/Logger.hpp"
#include "custom-opm-common/opm-common/opm/common/OpmLog/CounterLog.hpp"
#include "custom-opm-common/opm-common/opm/common/OpmLog/StreamLog.hpp"
#include "custom-opm-common/opm-common/opm/common/OpmLog/LogUtil.hpp"
*/

#include "custom-opm-parser/opm-parser/opm/parser/eclipse/Parser/Parser.hpp"


using namespace Opm;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(opm_parser_test, basicConstruction)
{
    Parser parser;
    std::string filename = "d:/Models/Statoil/testcase_juli_2011/data/TEST10K_FLT_LGR_NNC.DATA";
//    parser.newDeckFromFile(filename, Opm::ParseContext());
/*
    {
        std::string inline_json = "{\"key\": \"value\"}";
        Json::JsonObject parser(inline_json);
        parser.has_item("key");
        parser.has_item("keyX");
    }

    {
        ParserStringItemPtr itemString(new ParserStringItem(std::string("STRINGITEM1")));
        ParserRecordPtr record1(new ParserRecord());
        RawRecord rawRecord(" ' VALUE ' ");
        ParseContext parseContext;
        record1->addItem(itemString);
        //BOOST_CHECK_EQUAL(" VALUE ", deckRecord.getItem(0).get< std::string >(0));
    }

*/
    {
        const auto* input_deck = "RUNSPEC\n\n"
            "TITLE\n\n"
            "DIMENS\n10 10 10/\n"
            "EQLDIMS\n/\n";

        Parser parser;
        const auto deck = parser.newDeckFromString(input_deck, ParseContext());
        //BOOST_CHECK_EQUAL("untitled", deck->getKeyword("TITLE").getStringData().front());

    }
}



// TEST opm- common
TEST(opm_parser_test, opm_common_test)
{
/*
    Logger logger;
    std::ostringstream log_stream;
    std::shared_ptr<CounterLog> counter = std::make_shared<CounterLog>();
    std::shared_ptr<StreamLog> streamLog = std::make_shared<StreamLog>(log_stream, Log::MessageType::Warning);
    logger.hasBackend("NO");

    logger.addBackend("COUNTER", counter);
    logger.addBackend("STREAM", streamLog);
*/
/*
    BOOST_CHECK_EQUAL(true, logger.hasBackend("COUNTER"));
    BOOST_CHECK_EQUAL(true, logger.hasBackend("STREAM"));

    logger.addMessage(Log::MessageType::Error, "Error");
    logger.addMessage(Log::MessageType::Warning, "Warning");
    BOOST_CHECK_EQUAL(1U, counter->numMessages(Log::MessageType::Error));
    BOOST_CHECK_EQUAL(1U, counter->numMessages(Log::MessageType::Warning));
    BOOST_CHECK_EQUAL(0U, counter->numMessages(Log::MessageType::Info));

    BOOST_CHECK_EQUAL(log_stream.str(), "Warning\n");


    BOOST_CHECK_THROW(logger.getBackend<LogBackend>("No"), std::invalid_argument);
    {
        auto counter2 = logger.getBackend<CounterLog>("COUNTER");
        BOOST_CHECK_EQUAL(1U, counter2->numMessages(Log::MessageType::Warning));
        BOOST_CHECK_EQUAL(1U, counter2->numMessages(Log::MessageType::Error));
        BOOST_CHECK_EQUAL(0, counter2->numMessages(Log::MessageType::Info));
    }

    BOOST_CHECK_EQUAL(false, logger.removeBackend("NO-not-found"));
    BOOST_CHECK_EQUAL(true, logger.removeBackend("COUNTER"));
    BOOST_CHECK_EQUAL(false, logger.hasBackend("COUNTER"));
*/
}
