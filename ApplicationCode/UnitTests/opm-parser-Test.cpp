#include "gtest/gtest.h"

#include "opm/parser/eclipse/Parser/Parser.hpp"


using namespace Opm;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(opm_parser_test, basicConstruction)
{


    // Several filenames taken from Models/Statoil folder

/*
    std::string filename = "d:/Models/MRST/simple/SIMPLE.DATA";
    std::string filename = "d:/gitroot/opm-data/spe1/SPE1CASE1.DATA";
    std::string filename = "d:/Models/Statoil/Brillig/BRILLIG.DATA";
*/

 //   std::string filename = "d:/gitroot-magnesj/opm-parser/testdata/cases_with_issues/testcase_juli_2011/TEST10K_FLT_LGR_NNC.DATA";
/*
    std::string filename = "d:/Models/Statoil/1.2.0_Osesyd_segfault/BASEPRED6.DATA";    
    std::string filename = "d:/Models/Statoil/1.3.0_fault_assert_binary/BV-R2-11-0.DATA";
    std::string filename = "d:/Models/Statoil/Brillig/BRILLIG_FMTOUT.DATA";
    std::string filename = "d:/Models/Statoil/ceetron-case.tar/ceetron-case.tar";
    std::string filename = "d:/Models/Statoil/ceetron-case/.#R5_22X22_H25_C1_SW2_FV1.DATA";
    std::string filename = "d:/Models/Statoil/ceetron-case/R5_22X22_H25_C1_SW2_FV1.DATA";
    std::string filename = "d:/Models/Statoil/CO2_well_rst_error/E300_THERMAL_23_RS.DATA";
    std::string filename = "d:/Models/Statoil/E300_thermal_option/DualProperty/DUALPERM.DATA";
    std::string filename = "d:/Models/Statoil/E300_thermal_option/DualProperty/DUALPORO.DATA";
    std::string filename = "d:/Models/Statoil/HD_TEST/HD_TEST.DATA";
    std::string filename = "d:/Models/Statoil/HM_10/HM_10.DATA";
    std::string filename = "d:/Models/Statoil/LGC_TESTCASE2/LGC_TESTCASE2.DATA";
    std::string filename = "d:/Models/Statoil/MAGNE_DUALK2PORO_DPNUMz/DUALK2PORO_DPNUM.DATA";
    std::string filename = "d:/Models/Statoil/missingCorner_TP4093/TRAINING_FIXED_MOVE_INJ1.DATA";
    std::string filename = "d:/Models/Statoil/nnc_faults/TEST10K_FLT_LGR_NNC0P03_FAULTS1P0.DATA";
    std::string filename = "d:/Models/Statoil/NNCsDisconnected/TEST10K_FLT_LGR_NNC0P03_LGRNNC0P02_FAULTS0P1.DATA";
    std::string filename = "d:/Models/Statoil/NNCsDisconnected/TEST10K_FLT_LGR_NNC0P03_NNC0P02_FAULTS0P1.DATA";
    std::string filename = "d:/Models/Statoil/RegressionTests/1NotRun/TestCase_10K_MultipleTimestepsSameDay/TEST10K_FLT_LGR_NNC_TSTEP.DATA";
    std::string filename = "d:/Models/Statoil/RegressionTests/1NotRun/TestCase_10K_SingleTStep/TEST10K_FLT_LGR_NNC_TSTEP.DATA";
    std::string filename = "d:/Models/Statoil/RegressionTests/1NotRun/TestCase_DualPerm/DUALPERM.DATA";
    std::string filename = "d:/Models/Statoil/RegressionTests/1NotRun/TestCase_DualPoro/DUALPORO.DATA";
    std::string filename = "d:/Models/Statoil/RegressionTests/1NotRun/TestCase_Large_FirstLargeCase/R5_22X22_H25_C1_SW2_FV1.DATA";
    std::string filename = "d:/Models/Statoil/RegressionTests/1NotRun/TestCase_LGC/LGC_TESTCASE2.DATA";
    std::string filename = "d:/Models/Statoil/RegressionTests/1NotRun/TestCase_LGR_Amalg/TESTCASE_AMALG_LGR.DATA";
    std::string filename = "d:/Models/Statoil/RegressionTests/1NotRun/TestCase_Wells_HM_10/HM_10.DATA";
    std::string filename = "d:/Models/Statoil/RegressionTests/TestCase_10K_Complete/TEST10K_FLT_LGR_NNC.DATA";
    std::string filename = "d:/Models/Statoil/RegressionTests/tmp/TestCase_MultiRestartFile_CO2_well_rst_error/E300_THERMAL_23_RS.DATA";
    std::string filename = "d:/Models/Statoil/SmbS_ffm/REF161213.DATA";
    std::string filename = "d:/Models/Statoil/TEST_RKMFAULTS/TEST_RKMFAULTS.DATA";
    std::string filename = "d:/Models/Statoil/TEST10K_FLT_LGR_NNC_TSTEP/TEST10K_FLT_LGR_NNC_TSTEP.DATA";
    std::string filename = "d:/Models/Statoil/TestCase_10K_MultipleTimestepsSameDay/TEST10K_FLT_LGR_NNC_TSTEP.DATA";
    std::string filename = "d:/Models/Statoil/testcase_amalg_lgr/TESTCASE_AMALG_LGR.DATA";
    std::string filename = "d:/Models/Statoil/testcase_juli_2011/data/TEST10K_FLT_LGR_NNC.DATA";
    std::string filename = "d:/Models/Statoil/trainint_fixed_move/TRAINING_FIXED_MOVE_INJ1.DATA";
    std::string filename = "d:/Models/Statoil/troll_Ref2014/T07-4A-W2014-06.DATA";
*/



/*
    try
    {
    }
    catch (CException* e)
    {
    }
*/

/*
    ParseContext parseContext;
    parseContext.update(InputError::WARN);

    Opm::ParserPtr parser(new Opm::Parser());
    Opm::DeckConstPtr deck = parser->parseFile(filename, parseContext);

    Opm::EclipseState es(deck, parseContext);
    auto ep = es.get3DProperties();
    auto grid = es.getInputGrid();
*/


/*
    Parser parser;
    const auto deck = parser.newDeckFromFile(filename, Opm::ParseContext());
    const auto grid = Parser::parseGrid(*deck);
*/

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
