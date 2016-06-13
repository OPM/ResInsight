/*
  Copyright 2016 Statoil ASA.

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


#define BOOST_TEST_MODULE MessageContainerTest

#include <opm/common/utility/platform_dependent/disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <opm/common/utility/platform_dependent/reenable_warnings.h>

#include <opm/parser/eclipse/Parser/MessageContainer.hpp>


using namespace Opm;

BOOST_AUTO_TEST_CASE(TestIterator) {
    
    MessageContainer msgContainer;
    msgContainer.error("This is an error.");
    msgContainer.bug("This is a bug.", "dummy.log", 20);
    {       
        BOOST_CHECK_EQUAL("This is an error.", msgContainer.begin()->message);
        BOOST_CHECK_EQUAL("dummy.log", (msgContainer.end()-1)->location.filename);
        BOOST_CHECK_EQUAL(20U , (msgContainer.end()-1)->location.lineno);
    }
    
    MessageContainer msgList;
    msgList.debug("Debug");
    msgList.info("Info");
    msgList.warning("Warning");
    msgList.error("Error");
    msgList.problem("Problem");
    msgList.bug("Bug");
    msgList.note("Note");
    std::vector<std::string> msgString = {"Debug", "Info", "Warning", "Error", "Problem", "Bug", "Note"};
    int i = 0;
    for (const auto& msg : msgList) {
        BOOST_CHECK_EQUAL(msg.message, msgString[i]);
        i++;
    }
}

BOOST_AUTO_TEST_CASE(LocationImplicitConversion) {
    MessageContainer mc;
    mc.warning( "Warning" );
    mc.info( "Info", "filename", 10 );

    BOOST_CHECK( !mc.begin()->location );
    BOOST_CHECK( (mc.begin() + 1)->location );
    BOOST_CHECK_THROW( mc.info( "msg", "filename", 0 ), std::invalid_argument );
}


BOOST_AUTO_TEST_CASE(appendMessages) {
    MessageContainer msgContainer;
    MessageContainer msgList;

    msgContainer.error("Error: msgContainer.");
    BOOST_CHECK_EQUAL(1U, msgContainer.size());

    msgList.warning("Warning: msgList.");
    msgContainer.appendMessages(msgList);
    BOOST_CHECK_EQUAL(2U, msgContainer.size());

    BOOST_CHECK_EQUAL("Error: msgContainer.", msgContainer.begin()->message);
    BOOST_CHECK_EQUAL("Warning: msgList.", (msgContainer.end()-1)->message);
}
