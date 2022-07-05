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

#include <config.h>

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
#include <opm/common/OpmLog/KeywordLocation.hpp>

using namespace Opm;


BOOST_AUTO_TEST_CASE(DoLogging) {
    OpmLog::addMessage(Log::MessageType::Warning , "Warning1");
    OpmLog::addMessage(Log::MessageType::Warning , "Warning2");
}


BOOST_AUTO_TEST_CASE(Test_Format) {
    BOOST_CHECK_EQUAL( "There is an error here?\nIn file /path/to/file, line 100\n" , Log::fileMessage(KeywordLocation("Keyword", "/path/to/file" , 100) , "There is an error here?"));

    BOOST_CHECK_EQUAL( "\nError: This is the error" ,     Log::prefixMessage(Log::MessageType::Error , "This is the error"));
    BOOST_CHECK_EQUAL( "\nWarning: This is the warning" , Log::prefixMessage(Log::MessageType::Warning , "This is the warning"));
    BOOST_CHECK_EQUAL( "Info: This is the info" ,       Log::prefixMessage(Log::MessageType::Info , "This is the info"));
}



BOOST_AUTO_TEST_CASE(Test_Logger) {
    Logger logger;
    std::ostringstream log_stream;
    std::shared_ptr<CounterLog> counter = std::make_shared<CounterLog>();
    std::shared_ptr<StreamLog> streamLog = std::make_shared<StreamLog>( log_stream , Log::MessageType::Warning );
    BOOST_CHECK_THROW( StreamLog( "/non/existing/directory/logfile", Log::MessageType::Warning ) , std::runtime_error );
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
        BOOST_CHECK_EQUAL( 0U , counter2->numMessages( Log::MessageType::Info));
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

    void addMessageUnconditionally(int64_t messageType , const std::string& /* message */) override
    {
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
    counter.addMessage( Log::MessageType::Note , "This is a note");

    BOOST_CHECK_EQUAL(1U , counter.numMessages( Log::MessageType::Error ));
    BOOST_CHECK_EQUAL(1U , counter.numMessages( Log::MessageType::Warning ));
    BOOST_CHECK_EQUAL(0U , counter.numMessages( Log::MessageType::Info ));
    BOOST_CHECK_EQUAL(1U  , counter.numMessages( Log::MessageType::Note ));

    {
        int64_t not_enabled = 4096;
        int64_t not_power2  = 4095;

        BOOST_CHECK_EQUAL( 0U , counter.numMessages( not_enabled ));
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

        BOOST_CHECK_EQUAL( 1U , counter->numMessages(Log::MessageType::Error) );
        BOOST_CHECK_EQUAL( 1U , counter->numMessages(Log::MessageType::Warning) );
        BOOST_CHECK_EQUAL( 0U , counter->numMessages(Log::MessageType::Info) );
    }

    BOOST_CHECK_EQUAL( log_stream.str() , "Warning\n");
}



BOOST_AUTO_TEST_CASE(TestHelperFunctions)
{
    using namespace Log;

    // isPower2
    BOOST_CHECK(!isPower2(0));
    BOOST_CHECK(isPower2(1));
    BOOST_CHECK(isPower2(1 << 3));
    BOOST_CHECK(isPower2(1ul << 62));

    // fileMessage
    BOOST_CHECK_EQUAL(fileMessage(KeywordLocation("Keyword", "foo/bar", 1), "message"), "message\nIn file foo/bar, line 1\n");
    BOOST_CHECK_EQUAL(fileMessage(MessageType::Error, KeywordLocation("Keyword", "foo/bar", 1), "message"), "\nError: message\nIn file foo/bar, line 1\n");

    // prefixMessage
    BOOST_CHECK_EQUAL(prefixMessage(MessageType::Error, "message"), "\nError: message");
    BOOST_CHECK_EQUAL(prefixMessage(MessageType::Info, "message"), "Info: message");
    BOOST_CHECK_EQUAL(prefixMessage(MessageType::Note, "message"), "Note: message");

    // colorCode Message
    BOOST_CHECK_EQUAL(colorCodeMessage(MessageType::Info, "message"), "message");
    BOOST_CHECK_EQUAL(colorCodeMessage(MessageType::Warning, "message"), AnsiTerminalColors::blue_strong + "message" + AnsiTerminalColors::none);
    BOOST_CHECK_EQUAL(colorCodeMessage(MessageType::Error, "message"), AnsiTerminalColors::red_strong + "message" + AnsiTerminalColors::none);
}



BOOST_AUTO_TEST_CASE(TestOpmLogWithColors)
{
    OpmLog::removeAllBackends();

    std::ostringstream log_stream;

    {
        std::shared_ptr<CounterLog> counter = std::make_shared<CounterLog>();
        std::shared_ptr<StreamLog> streamLog = std::make_shared<StreamLog>(log_stream, Log::DefaultMessageTypes);
        BOOST_CHECK_EQUAL( false , OpmLog::hasBackend("NO"));
        OpmLog::addBackend("COUNTER" , counter);
        OpmLog::addBackend("STREAM" , streamLog);
        BOOST_CHECK_EQUAL( true , OpmLog::hasBackend("COUNTER"));
        BOOST_CHECK_EQUAL( true , OpmLog::hasBackend("STREAM"));

        streamLog->setMessageFormatter(std::make_shared<SimpleMessageFormatter>(false, true));
    }

    OpmLog::warning("Warning");
    OpmLog::error("Error");
    OpmLog::info("Info");
    OpmLog::bug("Bug");

    const std::string expected = Log::colorCodeMessage(Log::MessageType::Warning, "Warning") + "\n"
        + Log::colorCodeMessage(Log::MessageType::Error, "Error") + "\n"
        + Log::colorCodeMessage(Log::MessageType::Info, "Info") + "\n"
        + Log::colorCodeMessage(Log::MessageType::Bug, "Bug") + "\n";

    BOOST_CHECK_EQUAL(log_stream.str(), expected);

    {
        auto counter = OpmLog::getBackend<CounterLog>("COUNTER");

        BOOST_CHECK_EQUAL( 1U , counter->numMessages(Log::MessageType::Error) );
        BOOST_CHECK_EQUAL( 1U , counter->numMessages(Log::MessageType::Warning) );
        BOOST_CHECK_EQUAL( 1U , counter->numMessages(Log::MessageType::Info) );
        BOOST_CHECK_EQUAL( 1U , counter->numMessages(Log::MessageType::Bug) );
    }


    std::cout << log_stream.str() << std::endl;
}




BOOST_AUTO_TEST_CASE(TestOpmLogWithLimits)
{
    OpmLog::removeAllBackends();

    std::ostringstream log_stream1;
    std::ostringstream log_stream2;

    {
        std::shared_ptr<StreamLog> streamLog1 = std::make_shared<StreamLog>(log_stream1, Log::DefaultMessageTypes);
        std::shared_ptr<StreamLog> streamLog2 = std::make_shared<StreamLog>(log_stream2, Log::DefaultMessageTypes);
        OpmLog::addBackend("STREAM1" , streamLog1);
        OpmLog::addBackend("STREAM2" , streamLog2);
        BOOST_CHECK_EQUAL( true , OpmLog::hasBackend("STREAM1"));
        BOOST_CHECK_EQUAL( true , OpmLog::hasBackend("STREAM2"));

        streamLog1->setMessageFormatter(std::make_shared<SimpleMessageFormatter>(false, true));
        streamLog1->setMessageLimiter(std::make_shared<MessageLimiter>(2));
        streamLog2->setMessageFormatter(std::make_shared<SimpleMessageFormatter>(false, true));
        std::shared_ptr<MessageLimiter> lim(new MessageLimiter(MessageLimiter::NoLimit, {{ Log::MessageType::Warning, 2 }}));
        streamLog2->setMessageLimiter(lim); // no tag limit, but a warning category limit
    }

    const std::string tag = "ExampleTag";
    OpmLog::warning(tag, "Warning");
    OpmLog::error("Error");
    OpmLog::info("Info");
    OpmLog::bug("Bug");
    OpmLog::warning(tag, "Warning");
    OpmLog::warning(tag, "Warning");
    OpmLog::warning(tag, "Warning");

    const std::string expected1 = Log::colorCodeMessage(Log::MessageType::Warning, "Warning") + "\n"
        + Log::colorCodeMessage(Log::MessageType::Error, "Error") + "\n"
        + Log::colorCodeMessage(Log::MessageType::Info, "Info") + "\n"
        + Log::colorCodeMessage(Log::MessageType::Bug, "Bug") + "\n"
        + Log::colorCodeMessage(Log::MessageType::Warning, "Warning") + "\n"
        + Log::colorCodeMessage(Log::MessageType::Warning, "Message limit reached for message tag: " + tag) + "\n";

    BOOST_CHECK_EQUAL(log_stream1.str(), expected1);

    const std::string expected2 = Log::colorCodeMessage(Log::MessageType::Warning, "Warning") + "\n"
        + Log::colorCodeMessage(Log::MessageType::Error, "Error") + "\n"
        + Log::colorCodeMessage(Log::MessageType::Info, "Info") + "\n"
        + Log::colorCodeMessage(Log::MessageType::Bug, "Bug") + "\n"
        + Log::colorCodeMessage(Log::MessageType::Warning, "Warning") + "\n"
        + Log::colorCodeMessage(Log::MessageType::Warning, "Message limit reached for message category: \nWarning") + "\n";

    BOOST_CHECK_EQUAL(log_stream2.str(), expected2);

    std::cout << log_stream1.str() << std::endl;
    std::cout << log_stream2.str() << std::endl;
}




BOOST_AUTO_TEST_CASE(TestsetupSimpleLog)
{
    bool use_prefix = false;
    OpmLog::setupSimpleDefaultLogging(use_prefix);
    BOOST_CHECK_EQUAL(true, OpmLog::hasBackend("SimpleDefaultLog"));
}



BOOST_AUTO_TEST_CASE(TestFormat)
{
    OpmLog::removeAllBackends();
    std::ostringstream log_stream1;
    std::ostringstream log_stream2;
    std::ostringstream log_stream3;
    {
        std::shared_ptr<StreamLog> streamLog1 = std::make_shared<StreamLog>(log_stream1, Log::DefaultMessageTypes);
        std::shared_ptr<StreamLog> streamLog2 = std::make_shared<StreamLog>(log_stream2, Log::DefaultMessageTypes);
        std::shared_ptr<StreamLog> streamLog3 = std::make_shared<StreamLog>(log_stream3, Log::DefaultMessageTypes);
        OpmLog::addBackend("STREAM1" , streamLog1);
        OpmLog::addBackend("STREAM2" , streamLog2);
        OpmLog::addBackend("STREAM3" , streamLog3);
        BOOST_CHECK_EQUAL( true , OpmLog::hasBackend("STREAM1"));
        BOOST_CHECK_EQUAL( true , OpmLog::hasBackend("STREAM2"));
        BOOST_CHECK_EQUAL( true , OpmLog::hasBackend("STREAM3"));
        streamLog1->setMessageFormatter(std::make_shared<SimpleMessageFormatter>(false, true));
        streamLog2->setMessageFormatter(std::make_shared<SimpleMessageFormatter>(Log::MessageType::Info, true));
        streamLog3->setMessageFormatter(std::make_shared<SimpleMessageFormatter>(false));
    }

    OpmLog::warning("Warning");
    OpmLog::error("Error");
    OpmLog::info("Info");
    OpmLog::bug("Bug");

    const std::string expected1 = Log::colorCodeMessage(Log::MessageType::Warning, "Warning") + "\n"
        + Log::colorCodeMessage(Log::MessageType::Error, "Error") + "\n"
        + Log::colorCodeMessage(Log::MessageType::Info, "Info") + "\n"
        + Log::colorCodeMessage(Log::MessageType::Bug, "Bug") + "\n";

    const std::string expected2 = Log::colorCodeMessage(Log::MessageType::Warning, "Warning") + "\n"
        + Log::colorCodeMessage(Log::MessageType::Error, "Error") + "\n"
        + Log::colorCodeMessage(Log::MessageType::Info, "Info: Info") + "\n"
        + Log::colorCodeMessage(Log::MessageType::Bug, "Bug") + "\n";

    const std::string expected3 = Log::prefixMessage(Log::MessageType::Warning, "Warning") + "\n"
        + Log::prefixMessage(Log::MessageType::Error, "Error") + "\n"
        + "Info" + "\n"
        + Log::prefixMessage(Log::MessageType::Bug, "Bug") + "\n";

    BOOST_CHECK_EQUAL(log_stream1.str(), expected1);
    BOOST_CHECK_EQUAL(log_stream2.str(), expected2);
    BOOST_CHECK_EQUAL(log_stream3.str(), expected3);
}
