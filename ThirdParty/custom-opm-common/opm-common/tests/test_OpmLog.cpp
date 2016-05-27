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

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE LogTests

#include <opm/common/utility/platform_dependent/disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <opm/common/utility/platform_dependent/reenable_warnings.h>

#include <stdexcept>
#include <iostream>
#include <sstream>


#include <opm/common/OpmLog/OpmLog.hpp>
#include <opm/common/OpmLog/LogBackend.hpp>
#include <opm/common/OpmLog/CounterLog.hpp>
#include <opm/common/OpmLog/TimerLog.hpp>
#include <opm/common/OpmLog/StreamLog.hpp>
#include <opm/common/OpmLog/LogUtil.hpp>

using namespace Opm;


BOOST_AUTO_TEST_CASE(DoLogging) {
    OpmLog::addMessage(Log::MessageType::Warning , "Warning1");
    OpmLog::addMessage(Log::MessageType::Warning , "Warning2");
}


BOOST_AUTO_TEST_CASE(Test_Format) {
    BOOST_CHECK_EQUAL( "/path/to/file:100: There is a mild fuckup here?" , Log::fileMessage("/path/to/file" , 100 , "There is a mild fuckup here?"));

    BOOST_CHECK_EQUAL( "error: This is the error" ,     Log::prefixMessage(Log::MessageType::Error , "This is the error"));
    BOOST_CHECK_EQUAL( "warning: This is the warning" , Log::prefixMessage(Log::MessageType::Warning , "This is the warning"));
    BOOST_CHECK_EQUAL( "info: This is the info" ,       Log::prefixMessage(Log::MessageType::Info , "This is the info"));
}



BOOST_AUTO_TEST_CASE(Test_AbstractBackend) {
    int64_t mask = 1+4+16;
    LogBackend backend(mask);

    BOOST_CHECK_EQUAL(false , backend.includeMessage(0 ));
    BOOST_CHECK_EQUAL(true  , backend.includeMessage(1 ));
    BOOST_CHECK_EQUAL(false , backend.includeMessage(2 ));
    BOOST_CHECK_EQUAL(true  , backend.includeMessage(4 ));
    BOOST_CHECK_EQUAL(false , backend.includeMessage(8 ));
    BOOST_CHECK_EQUAL(true  , backend.includeMessage(16 ));

    BOOST_CHECK_EQUAL(false, backend.includeMessage(6 ));
    BOOST_CHECK_EQUAL(true , backend.includeMessage(5 ));
}



BOOST_AUTO_TEST_CASE(Test_Logger) {
    Logger logger;
    std::ostringstream log_stream;
    std::shared_ptr<CounterLog> counter = std::make_shared<CounterLog>();
    std::shared_ptr<StreamLog> streamLog = std::make_shared<StreamLog>( log_stream , Log::MessageType::Warning );
    BOOST_CHECK_EQUAL( false , logger.hasBackend("NO"));

    logger.addBackend("COUNTER" , counter);
    logger.addBackend("STREAM" , streamLog);
    BOOST_CHECK_EQUAL( true , logger.hasBackend("COUNTER"));
    BOOST_CHECK_EQUAL( true , logger.hasBackend("STREAM"));

    logger.addMessage( Log::MessageType::Error , "Error");
    logger.addMessage( Log::MessageType::Warning , "Warning");
    BOOST_CHECK_EQUAL( 1U , counter->numMessages(Log::MessageType::Error) );
    BOOST_CHECK_EQUAL( 1U , counter->numMessages(Log::MessageType::Warning) );
    BOOST_CHECK_EQUAL( 0U , counter->numMessages(Log::MessageType::Info) );

    BOOST_CHECK_EQUAL( log_stream.str() , "Warning\n");


    BOOST_CHECK_THROW( logger.getBackend<LogBackend>("No") , std::invalid_argument );
    {
        auto counter2 = logger.getBackend<CounterLog>("COUNTER");
        BOOST_CHECK_EQUAL( 1U , counter2->numMessages( Log::MessageType::Warning));
        BOOST_CHECK_EQUAL( 1U , counter2->numMessages( Log::MessageType::Error));
        BOOST_CHECK_EQUAL( 0  , counter2->numMessages( Log::MessageType::Info));
    }

    BOOST_CHECK_EQUAL( false , logger.removeBackend("NO-not-found"));
    BOOST_CHECK_EQUAL( true , logger.removeBackend("COUNTER"));
    BOOST_CHECK_EQUAL( false , logger.hasBackend("COUNTER") );

    {
        auto stream2 = logger.popBackend<StreamLog>("STREAM");
        BOOST_CHECK_EQUAL( false , logger.hasBackend("STREAM") );
        BOOST_CHECK_THROW( logger.popBackend<StreamLog>("STREAM") , std::invalid_argument );
    }
}


BOOST_AUTO_TEST_CASE(LoggerAddTypes_PowerOf2) {
    Logger logger;
    int64_t not_power_of2 = 13;
    int64_t power_of2 = 4096;

    BOOST_CHECK_THROW( logger.addMessageType( not_power_of2 , "Prefix") , std::invalid_argument);
    BOOST_CHECK_THROW( logger.enabledMessageType( not_power_of2 ) , std::invalid_argument);

    logger.addMessageType( power_of2 , "Prefix");
    BOOST_CHECK( logger.enabledMessageType( power_of2 ));
    BOOST_CHECK_EQUAL( false , logger.enabledMessageType( 2*power_of2 ));
}


class TestLog: public LogBackend {
public:
    TestLog( int64_t messageMask ) : LogBackend( messageMask )
    {
        m_defaultMessages = 0;
        m_specialMessages = 0;
    }

    void addMessage(int64_t messageType , const std::string& /* message */) {
        if (messageType & Log::DefaultMessageTypes)
            m_defaultMessages +=1;
        else
            m_specialMessages += 1;
    }

    int m_defaultMessages;
    int m_specialMessages;
};
/*
  Testing that the logger frontend does not let unknown message types
  pass through; even though the backend has shown interest in the
  phony 4096 messagetype.
*/

BOOST_AUTO_TEST_CASE(LoggerMasksTypes) {
    Logger logger;
    int64_t power_of2 = 4096;

    std::shared_ptr<TestLog> testLog = std::make_shared<TestLog>(Log::DefaultMessageTypes + power_of2);
    logger.addBackend("TEST" , testLog);
    BOOST_CHECK_EQUAL( false , logger.enabledMessageType( power_of2 ));

    logger.addMessage( Log::MessageType::Error , "Error");
    logger.addMessage( Log::MessageType::Warning , "Warning");
    logger.addMessage( Log::MessageType::Info , "Info");

    BOOST_CHECK_THROW( logger.addMessage( power_of2 , "Blocked message") , std::invalid_argument );
    BOOST_CHECK_EQUAL( testLog->m_defaultMessages , 3 );
    BOOST_CHECK_EQUAL( testLog->m_specialMessages , 0 );

    logger.addMessageType( power_of2 , "Phony");
    logger.addMessage( power_of2 , "Passing through");
    BOOST_CHECK_EQUAL( testLog->m_specialMessages , 1 );
}





BOOST_AUTO_TEST_CASE(LoggerDefaultTypesEnabled) {
    Logger logger;
    BOOST_CHECK_EQUAL( logger.enabledMessageTypes() , Log::DefaultMessageTypes);
}

BOOST_AUTO_TEST_CASE( CounterLogTesting) {
    CounterLog counter(Log::DefaultMessageTypes);

    counter.addMessage( Log::MessageType::Error , "This is an error ...");
    counter.addMessage( Log::MessageType::Warning , "This is a warning");

    BOOST_CHECK_EQUAL(1U , counter.numMessages( Log::MessageType::Error ));
    BOOST_CHECK_EQUAL(1U , counter.numMessages( Log::MessageType::Warning ));
    BOOST_CHECK_EQUAL(0  , counter.numMessages( Log::MessageType::Info ));

    {
        int64_t not_enabled = 4096;
        int64_t not_power2  = 4095;

        BOOST_CHECK_EQUAL( 0 , counter.numMessages( not_enabled ));
        BOOST_CHECK_THROW( counter.numMessages( not_power2 ) , std::invalid_argument);
    }
}

BOOST_AUTO_TEST_CASE(TestTimerLog) {
    Logger logger;
    std::ostringstream sstream;
    std::shared_ptr<TimerLog> timer = std::make_shared<TimerLog>(sstream);
    logger.addBackend( "TIMER" , timer );
    logger.addMessageType( TimerLog::StartTimer , "Start");
    logger.addMessageType( TimerLog::StopTimer , "Stop");

    logger.addMessage( TimerLog::StartTimer , "");
    logger.addMessage( TimerLog::StopTimer , "This was fast");
    std::cout << sstream.str() << std::endl;
}


/*****************************************************************/
void initLogger(std::ostringstream& log_stream);

void initLogger(std::ostringstream& log_stream) {
    std::shared_ptr<CounterLog> counter = std::make_shared<CounterLog>();
    std::shared_ptr<StreamLog> streamLog = std::make_shared<StreamLog>( log_stream , Log::MessageType::Warning );

    BOOST_CHECK_EQUAL( false , OpmLog::hasBackend("NO"));

    OpmLog::addBackend("COUNTER" , counter);
    OpmLog::addBackend("STREAM" , streamLog);
    BOOST_CHECK_EQUAL( true , OpmLog::hasBackend("COUNTER"));
    BOOST_CHECK_EQUAL( true , OpmLog::hasBackend("STREAM"));
}



BOOST_AUTO_TEST_CASE(TestOpmLog) {
    std::ostringstream log_stream;

    initLogger(log_stream);

    OpmLog::addMessage( Log::MessageType::Warning , "Warning");
    OpmLog::addMessage( Log::MessageType::Error , "Error");

    {
        auto counter = OpmLog::getBackend<CounterLog>("COUNTER");

        BOOST_CHECK_EQUAL( 1 , counter->numMessages(Log::MessageType::Error) );
        BOOST_CHECK_EQUAL( 1 , counter->numMessages(Log::MessageType::Warning) );
        BOOST_CHECK_EQUAL( 0 , counter->numMessages(Log::MessageType::Info) );
    }

    BOOST_CHECK_EQUAL( log_stream.str() , "Warning\n");
}
