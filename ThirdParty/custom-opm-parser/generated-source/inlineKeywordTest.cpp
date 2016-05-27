#define BOOST_TEST_MODULE ParserRecordTests
#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include <memory>
#include <opm/json/JsonObject.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords.hpp>
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
#include <opm/parser/eclipse/Parser/ParserItem.hpp>
#include <opm/parser/eclipse/Parser/ParserIntItem.hpp>
#include <opm/parser/eclipse/Parser/ParserStringItem.hpp>
#include <opm/parser/eclipse/Parser/ParserDoubleItem.hpp>
#include <opm/parser/eclipse/Parser/ParserRecord.hpp>
#include <opm/parser/eclipse/Units/UnitSystem.hpp>
using namespace Opm;
std::shared_ptr<UnitSystem> unitSystem( UnitSystem::newMETRIC() );
BOOST_AUTO_TEST_CASE(TESTACTDIMSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/A/ACTDIMS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::ACTDIMS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTACTNUMKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/A/ACTNUM";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::ACTNUM inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTADDKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/A/ADD";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::ADD inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTADDREGKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/A/ADDREG";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::ADDREG inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTADSALNODKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/A/ADSALNOD";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::ADSALNOD inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTALLKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/A/ALL";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::ALL inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTAPIKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/A/API";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::API inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTAQUCONKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/A/AQUCON";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::AQUCON inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTAQUDIMSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/A/AQUDIMS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::AQUDIMS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTAQUNUMKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/A/AQUNUM";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::AQUNUM inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTBLOCK_PROBEKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/B/BLOCK_PROBE";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::BLOCK_PROBE inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTBLOCK_PROBE300Keyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/001_Eclipse300/B/BLOCK_PROBE300";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::BLOCK_PROBE300 inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTBOXKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/B/BOX";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::BOX inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTCOMPDATKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/C/COMPDAT";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::COMPDAT inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTCOMPLUMPKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/C/COMPLUMP";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::COMPLUMP inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTCOMPORDKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/C/COMPORD";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::COMPORD inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTCOMPSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/001_Eclipse300/C/COMPS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::COMPS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTCOMPSEGSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/C/COMPSEGS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::COMPSEGS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTCONNECTION_PROBEKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/C/CONNECTION_PROBE";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::CONNECTION_PROBE inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTCOORDKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/C/COORD";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::COORD inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTCOPYKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/C/COPY";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::COPY inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTCOPYREGKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/C/COPYREG";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::COPYREG inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTCPRKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/C/CPR";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::CPR inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTCREFKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/001_Eclipse300/C/CREF";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::CREF inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTCREFSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/001_Eclipse300/C/CREFS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::CREFS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTDATEKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/D/DATE";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::DATE inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTDATESKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/D/DATES";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::DATES inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTDENSITYKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/D/DENSITY";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::DENSITY inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTDEPTHKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/D/DEPTH";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::DEPTH inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTDEPTHZKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/D/DEPTHZ";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::DEPTHZ inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTDIMENSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/D/DIMENS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::DIMENS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTDISGASKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/D/DISGAS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::DISGAS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTDREFKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/001_Eclipse300/D/DREF";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::DREF inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTDREFSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/001_Eclipse300/D/DREFS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::DREFS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTDRSDTKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/D/DRSDT";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::DRSDT inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTDRVDTKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/D/DRVDT";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::DRVDT inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTDXKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/D/DX";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::DX inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTDXVKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/D/DXV";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::DXV inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTDYKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/D/DY";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::DY inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTDYVKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/D/DYV";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::DYV inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTDZKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/D/DZ";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::DZ inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTDZVKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/001_Eclipse300/D/DZV";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::DZV inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTECHOKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/E/ECHO";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::ECHO inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTEDITKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/E/EDIT";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::EDIT inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTEDITNNCKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/E/EDITNNC";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::EDITNNC inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTEHYSTRKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/E/EHYSTR";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::EHYSTR inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTENDKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/E/END";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::END inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTENDBOXKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/E/ENDBOX";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::ENDBOX inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTENDINCKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/E/ENDINC";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::ENDINC inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTENDNUMKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/E/ENDNUM";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::ENDNUM inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTENDPOINT_SPECIFIERSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/E/ENDPOINT_SPECIFIERS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::ENDPOINT_SPECIFIERS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTENDSCALEKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/E/ENDSCALE";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::ENDSCALE inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTENDSKIPKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/E/ENDSKIP";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::ENDSKIP inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTENKRVDKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/E/ENKRVD";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::ENKRVD inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTENPTVDKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/E/ENPTVD";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::ENPTVD inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTEQLDIMSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/E/EQLDIMS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::EQLDIMS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTEQLNUMKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/E/EQLNUM";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::EQLNUM inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTEQLOPTSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/E/EQLOPTS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::EQLOPTS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTEQUALREGKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/E/EQUALREG";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::EQUALREG inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTEQUALSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/E/EQUALS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::EQUALS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTEQUILKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/E/EQUIL";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::EQUIL inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTEXCELKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/E/EXCEL";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::EXCEL inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTEXTRAPMSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/E/EXTRAPMS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::EXTRAPMS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTFAULTDIMKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/F/FAULTDIM";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::FAULTDIM inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTFAULTSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/F/FAULTS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::FAULTS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTFIELDKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/F/FIELD";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::FIELD inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTFIELD_PROBEKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/F/FIELD_PROBE";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::FIELD_PROBE inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTFILLEPSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/F/FILLEPS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::FILLEPS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTFIPNUMKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/F/FIPNUM";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::FIPNUM inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTFLUXNUMKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/F/FLUXNUM";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::FLUXNUM inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTFMTINKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/F/FMTIN";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::FMTIN inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTFMTOUTKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/F/FMTOUT";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::FMTOUT inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTFULLIMPKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/F/FULLIMP";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::FULLIMP inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTGASKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/G/GAS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::GAS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTGASVISCTKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/001_Eclipse300/G/GASVISCT";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::GASVISCT inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTGCOMPIDXKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/900_OPM/G/GCOMPIDX";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::GCOMPIDX inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTGCONINJEKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/G/GCONINJE";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::GCONINJE inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTGCONPRODKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/G/GCONPROD";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::GCONPROD inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTGDORIENTKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/G/GDORIENT";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::GDORIENT inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTGECONKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/G/GECON";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::GECON inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTGEFACKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/G/GEFAC";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::GEFAC inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTGRIDKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/G/GRID";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::GRID inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTGRIDFILEKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/G/GRIDFILE";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::GRIDFILE inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTGRIDOPTSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/G/GRIDOPTS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::GRIDOPTS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTGRIDUNITKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/G/GRIDUNIT";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::GRIDUNIT inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTGROUP_PROBEKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/G/GROUP_PROBE";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::GROUP_PROBE inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTGRUPNETKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/G/GRUPNET";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::GRUPNET inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTGRUPTREEKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/G/GRUPTREE";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::GRUPTREE inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTIMBNUMKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/I/IMBNUM";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::IMBNUM inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTIMKRVDKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/I/IMKRVD";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::IMKRVD inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTIMPESKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/I/IMPES";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::IMPES inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTIMPTVDKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/I/IMPTVD";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::IMPTVD inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTINCLUDEKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/I/INCLUDE";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::INCLUDE inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTINITKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/I/INIT";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::INIT inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTIPCGKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/I/IPCG";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::IPCG inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTIPCWKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/I/IPCW";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::IPCW inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTISGCRKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/I/ISGCR";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::ISGCR inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTISGLKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/I/ISGL";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::ISGL inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTISGUKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/I/ISGU";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::ISGU inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTISOGCRKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/I/ISOGCR";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::ISOGCR inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTISOWCRKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/I/ISOWCR";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::ISOWCR inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTISWCRKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/I/ISWCR";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::ISWCR inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTISWLKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/I/ISWL";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::ISWL inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTISWUKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/I/ISWU";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::ISWU inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTMAPAXESKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/M/MAPAXES";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::MAPAXES inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTMAPUNITSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/M/MAPUNITS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::MAPUNITS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTMAXVALUEKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/M/MAXVALUE";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::MAXVALUE inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTMEMORYKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/M/MEMORY";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::MEMORY inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTMESSAGESKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/M/MESSAGES";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::MESSAGES inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTMETRICKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/M/METRIC";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::METRIC inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTMINPVKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/M/MINPV";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::MINPV inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTMINPVFILKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/900_OPM/M/MINPVFIL";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::MINPVFIL inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTMINVALUEKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/M/MINVALUE";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::MINVALUE inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTMISCKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/M/MISC";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::MISC inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTMISCIBLEKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/M/MISCIBLE";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::MISCIBLE inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTMISCNUMKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/M/MISCNUM";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::MISCNUM inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTMONITORKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/M/MONITOR";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::MONITOR inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTMSFNKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/M/MSFN";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::MSFN inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTMSGFILEKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/M/MSGFILE";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::MSGFILE inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTMULTFLTKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/M/MULTFLT";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::MULTFLT inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTMULTIPLYKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/M/MULTIPLY";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::MULTIPLY inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTMULTIREGKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/M/MULTIREG";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::MULTIREG inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTMULTNUMKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/M/MULTNUM";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::MULTNUM inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTMULTPVKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/M/MULTPV";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::MULTPV inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTMULTREGTKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/M/MULTREGT";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::MULTREGT inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTMULT_XYZKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/M/MULT_XYZ";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::MULT_XYZ inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTMWKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/001_Eclipse300/M/MW";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::MW inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTMWSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/001_Eclipse300/M/MWS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::MWS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTNETBALANKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/N/NETBALAN";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::NETBALAN inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTNEWTRANKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/N/NEWTRAN";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::NEWTRAN inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTNEXTSTEPKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/N/NEXTSTEP";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::NEXTSTEP inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTNNCKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/N/NNC";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::NNC inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTNOCASCKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/N/NOCASC";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::NOCASC inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTNOECHOKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/N/NOECHO";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::NOECHO inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTNOGGFKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/N/NOGGF";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::NOGGF inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTNOGRAVKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/002_Frontsim/N/NOGRAV";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::NOGRAV inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTNOINSPECKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/N/NOINSPEC";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::NOINSPEC inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTNOMONITOKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/N/NOMONITO";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::NOMONITO inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTNONNCKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/N/NONNC";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::NONNC inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTNORSSPECKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/N/NORSSPEC";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::NORSSPEC inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTNOSIMKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/N/NOSIM";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::NOSIM inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTNSTACKKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/N/NSTACK";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::NSTACK inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTNTGKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/N/NTG";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::NTG inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTNUMRESKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/N/NUMRES";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::NUMRES inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTNUPCOLKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/N/NUPCOL";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::NUPCOL inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTOCOMPIDXKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/900_OPM/O/OCOMPIDX";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::OCOMPIDX inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTOILKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/O/OIL";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::OIL inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTOILCOMPRKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/001_Eclipse300/O/OILCOMPR";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::OILCOMPR inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTOILMWKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/001_Eclipse300/O/OILMW";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::OILMW inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTOILVISCTKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/O/OILVISCT";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::OILVISCT inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTOILVTIMKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/001_Eclipse300/O/OILVTIM";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::OILVTIM inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTOLDTRANKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/O/OLDTRAN";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::OLDTRAN inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTOPTIONSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/O/OPTIONS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::OPTIONS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPARALLELKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PARALLEL";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PARALLEL inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPATHSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PATHS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PATHS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPBVDKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PBVD";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PBVD inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPCGKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PCG";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PCG inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPERFORMANCE_PROBEKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PERFORMANCE_PROBE";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PERFORMANCE_PROBE inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPERMXKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PERMX";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PERMX inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPERMXYKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PERMXY";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PERMXY inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPERMYKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PERMY";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PERMY inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPERMYZKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PERMYZ";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PERMYZ inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPERMZKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PERMZ";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PERMZ inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPERMZXKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PERMZX";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PERMZX inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPIMTDIMSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PIMTDIMS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PIMTDIMS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPIMULTABKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PIMULTAB";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PIMULTAB inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPINCHKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PINCH";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PINCH inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPLMIXPARKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PLMIXPAR";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PLMIXPAR inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPLYADSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PLYADS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PLYADS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPLYADSSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PLYADSS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PLYADSS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPLYDHFLFKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PLYDHFLF";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PLYDHFLF inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPLYMAXKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PLYMAX";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PLYMAX inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPLYROCKKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PLYROCK";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PLYROCK inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPLYSHEARKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PLYSHEAR";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PLYSHEAR inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPLYSHLOGKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PLYSHLOG";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PLYSHLOG inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPLYVISCKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PLYVISC";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PLYVISC inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPMISCKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PMISC";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PMISC inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPOLYMERKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/POLYMER";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::POLYMER inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPOROKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PORO";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PORO inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPORVKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PORV";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PORV inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPREFKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/001_Eclipse300/P/PREF";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PREF inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPREFSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/001_Eclipse300/P/PREFS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PREFS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPRESSUREKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PRESSURE";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PRESSURE inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPROPSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PROPS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PROPS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPVCDOKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PVCDO";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PVCDO inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPVDGKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PVDG";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PVDG inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPVDOKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PVDO";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PVDO inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPVDSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PVDS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PVDS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPVTGKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PVTG";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PVTG inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPVTNUMKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PVTNUM";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PVTNUM inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPVTOKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PVTO";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PVTO inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTPVTWKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/P/PVTW";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::PVTW inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTRADFIN4Keyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/R/RADFIN4";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::RADFIN4 inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTREGDIMSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/R/REGDIMS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::REGDIMS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTREGIONSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/R/REGIONS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::REGIONS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTREGION_PROBEKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/R/REGION_PROBE";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::REGION_PROBE inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTRESTARTKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/R/RESTART";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::RESTART inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTRKTRMDIRKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/R/RKTRMDIR";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::RKTRMDIR inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTROCKKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/R/ROCK";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::ROCK inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTROCKCOMPKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/R/ROCKCOMP";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::ROCKCOMP inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTROCKOPTSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/R/ROCKOPTS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::ROCKOPTS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTROCKTABKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/R/ROCKTAB";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::ROCKTAB inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTRPTGRIDKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/R/RPTGRID";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::RPTGRID inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTRPTONLYKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/R/RPTONLY";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::RPTONLY inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTRPTONLYOKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/R/RPTONLYO";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::RPTONLYO inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTRPTPROPSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/R/RPTPROPS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::RPTPROPS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTRPTRSTKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/R/RPTRST";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::RPTRST inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTRPTRUNSPKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/R/RPTRUNSP";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::RPTRUNSP inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTRPTSCHEDKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/R/RPTSCHED";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::RPTSCHED inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTRPTSOLKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/R/RPTSOL";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::RPTSOL inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTRSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/R/RS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::RS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTRSVDKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/R/RSVD";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::RSVD inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTRTEMPVDKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/R/RTEMPVD";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::RTEMPVD inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTRUNSPECKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/R/RUNSPEC";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::RUNSPEC inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTRUNSUMKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/R/RUNSUM";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::RUNSUM inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTRVKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/R/RV";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::RV inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTRVVDKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/R/RVVD";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::RVVD inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSATNUMKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SATNUM";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SATNUM inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSATOPTSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SATOPTS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SATOPTS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSAVEKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SAVE";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SAVE inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSCALECRSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SCALECRS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SCALECRS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSCHEDULEKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SCHEDULE";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SCHEDULE inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSDENSITYKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SDENSITY";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SDENSITY inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSEPARATEKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SEPARATE";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SEPARATE inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSGASKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SGAS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SGAS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSGCRKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SGCR";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SGCR inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSGCWMISKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SGCWMIS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SGCWMIS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSGFNKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SGFN";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SGFN inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSGLKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SGL";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SGL inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSGOFKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SGOF";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SGOF inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSGUKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SGU";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SGU inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSGWFNKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SGWFN";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SGWFN inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSHRATEKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SHRATE";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SHRATE inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSKIPKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SKIP";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SKIP inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSKIP100Keyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SKIP100";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SKIP100 inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSKIP300Keyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SKIP300";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SKIP300 inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSKIPRESTKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SKIPREST";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SKIPREST inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSLGOFKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SLGOF";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SLGOF inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSMRYDIMSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SMRYDIMS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SMRYDIMS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSOF2Keyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SOF2";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SOF2 inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSOF3Keyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SOF3";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SOF3 inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSOGCRKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SOGCR";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SOGCR inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSOILKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SOIL";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SOIL inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSOLUTIONKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SOLUTION";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SOLUTION inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSOLVENTKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SOLVENT";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SOLVENT inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSORWMISKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SORWMIS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SORWMIS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSOWCRKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SOWCR";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SOWCR inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSPECGRIDKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SPECGRID";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SPECGRID inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSPECHEATKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SPECHEAT";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SPECHEAT inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSPECROCKKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SPECROCK";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SPECROCK inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSSFNKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SSFN";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SSFN inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSSOLKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SSOL";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SSOL inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSTARTKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/START";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::START inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSTCONDKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/001_Eclipse300/S/STCOND";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::STCOND inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSTONE1Keyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/STONE1";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::STONE1 inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSTONE1EXKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/STONE1EX";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::STONE1EX inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSUMMARYKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SUMMARY";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SUMMARY inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSUMTHINKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SUMTHIN";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SUMTHIN inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSWATKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SWAT";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SWAT inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSWATINITKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SWATINIT";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SWATINIT inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSWCRKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SWCR";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SWCR inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSWFNKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SWFN";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SWFN inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSWLKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SWL";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SWL inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSWOFKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SWOF";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SWOF inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTSWUKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/S/SWU";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::SWU inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTTABDIMSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/T/TABDIMS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::TABDIMS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTTEMPKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/T/TEMP";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::TEMP inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTTEMPIKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/001_Eclipse300/T/TEMPI";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::TEMPI inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTTEMPVDKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/001_Eclipse300/T/TEMPVD";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::TEMPVD inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTTHCONRKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/T/THCONR";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::THCONR inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTTHERMALKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/001_Eclipse300/T/THERMAL";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::THERMAL inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTTHERMEX1Keyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/001_Eclipse300/T/THERMEX1";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::THERMEX1 inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTTHPRESKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/T/THPRES";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::THPRES inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTTITLEKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/T/TITLE";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::TITLE inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTTLMIXPARKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/T/TLMIXPAR";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::TLMIXPAR inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTTLPMIXPAKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/900_OPM/T/TLPMIXPA";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::TLPMIXPA inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTTOPSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/T/TOPS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::TOPS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTTRACERKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/T/TRACER";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::TRACER inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTTRACERSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/T/TRACERS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::TRACERS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTTRANXKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/T/TRANX";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::TRANX inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTTRANYKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/T/TRANY";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::TRANY inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTTRANZKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/T/TRANZ";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::TRANZ inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTTREFKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/001_Eclipse300/T/TREF";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::TREF inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTTREFSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/001_Eclipse300/T/TREFS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::TREFS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTTSTEPKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/T/TSTEP";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::TSTEP inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTTUNINGKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/T/TUNING";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::TUNING inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTTVDPKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/T/TVDP";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::TVDP inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTUDADIMSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/U/UDADIMS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::UDADIMS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTUDQDIMSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/U/UDQDIMS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::UDQDIMS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTUNIFINKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/U/UNIFIN";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::UNIFIN inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTUNIFOUTKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/U/UNIFOUT";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::UNIFOUT inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTVAPOILKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/V/VAPOIL";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::VAPOIL inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTVAPPARSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/V/VAPPARS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::VAPPARS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTVFPIDIMSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/V/VFPIDIMS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::VFPIDIMS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTVFPINJKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/V/VFPINJ";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::VFPINJ inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTVFPPDIMSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/V/VFPPDIMS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::VFPPDIMS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTVFPPRODKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/V/VFPPROD";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::VFPPROD inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTVISCREFKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/V/VISCREF";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::VISCREF inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTWATDENTKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/001_Eclipse300/W/WATDENT";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::WATDENT inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTWATERKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/W/WATER";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::WATER inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTWATVISCTKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/W/WATVISCT";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::WATVISCT inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTWCONHISTKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/W/WCONHIST";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::WCONHIST inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTWCONINJKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/W/WCONINJ";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::WCONINJ inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTWCONINJEKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/W/WCONINJE";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::WCONINJE inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTWCONINJHKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/W/WCONINJH";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::WCONINJH inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTWCONPRODKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/W/WCONPROD";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::WCONPROD inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTWELLDIMSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/W/WELLDIMS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::WELLDIMS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTWELL_PROBEKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/W/WELL_PROBE";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::WELL_PROBE inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTWELOPENKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/W/WELOPEN";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::WELOPEN inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTWELSEGSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/W/WELSEGS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::WELSEGS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTWELSPECSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/W/WELSPECS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::WELSPECS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTWELTARGKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/W/WELTARG";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::WELTARG inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTWGRUPCONKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/W/WGRUPCON";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::WGRUPCON inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTWHISTCTLKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/W/WHISTCTL";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::WHISTCTL inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTWPAVEKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/W/WPAVE";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::WPAVE inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTWPIMULTKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/W/WPIMULT";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::WPIMULT inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTWPITABKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/W/WPITAB";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::WPITAB inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTWPOLYMERKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/W/WPOLYMER";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::WPOLYMER inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTWRFTKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/W/WRFT";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::WRFT inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTWRFTPLTKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/W/WRFTPLT";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::WRFTPLT inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTWSEGDIMSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/W/WSEGDIMS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::WSEGDIMS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTWSOLVENTKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/W/WSOLVENT";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::WSOLVENT inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTWTEMPKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/W/WTEMP";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::WTEMP inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTWTESTKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/W/WTEST";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::WTEST inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTWTRACERKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/W/WTRACER";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::WTRACER inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTZCORNKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/Z/ZCORN";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::ZCORN inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTZFACT1Keyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/001_Eclipse300/Z/ZFACT1";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::ZFACT1 inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTZFACT1SKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/001_Eclipse300/Z/ZFACT1S";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::ZFACT1S inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTZFACTORKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/001_Eclipse300/Z/ZFACTOR";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::ZFACTOR inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTZFACTORSKeyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/001_Eclipse300/Z/ZFACTORS";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::ZFACTORS inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

BOOST_AUTO_TEST_CASE(TESTZIPPY2Keyword) {
    std::string jsonFile = "D:/gitroot-magnesj/opm-parser/opm/parser/share/keywords/000_Eclipse100/Z/ZIPPY2";
    boost::filesystem::path jsonPath( jsonFile );
    Json::JsonObject jsonConfig( jsonPath );
    ParserKeyword jsonKeyword(jsonConfig);
    ParserKeywords::ZIPPY2 inlineKeyword;
    BOOST_CHECK( jsonKeyword.equal( inlineKeyword ));
    if (jsonKeyword.hasDimension()) {
        ParserRecordConstPtr parserRecord = jsonKeyword.getRecord(0);
        for (size_t i=0; i < parserRecord->size(); i++){ 
            ParserItemConstPtr item = parserRecord->get( i );
            for (size_t j=0; j < item->numDimensions(); j++) {
                std::string dimString = item->getDimension(j);
                BOOST_CHECK_NO_THROW( unitSystem->getNewDimension( dimString ));
             }
        }
    }
}

