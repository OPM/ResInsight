/*
  Copyright (C) 2015 by Statoil ASA

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

#define BOOST_TEST_MODULE ColumnSchemaTests

#include <boost/test/unit_test.hpp>


#include <opm/input/eclipse/EclipseState/Tables/ColumnSchema.hpp>

using namespace Opm;


BOOST_AUTO_TEST_CASE( CreateTest ) {
    ColumnSchema schema("Name" , Table::INCREASING , Table::DEFAULT_NONE);
    BOOST_CHECK_EQUAL( schema.name() , "Name");
    BOOST_CHECK_THROW( schema.getDefaultValue() , std::invalid_argument );
}


BOOST_AUTO_TEST_CASE( CreateDefaultConst ) {
    ColumnSchema schema("Name" , Table::INCREASING , 1.76);
    BOOST_CHECK_EQUAL( schema.name() , "Name");
    BOOST_CHECK_EQUAL( schema.getDefaultMode() , Table::DEFAULT_CONST );
    BOOST_CHECK_EQUAL( schema.getDefaultValue() , 1.76 );
}


BOOST_AUTO_TEST_CASE( TestOrder) {
    {
        ColumnSchema schema("Name" , Table::INCREASING , Table::DEFAULT_NONE);
        BOOST_CHECK_EQUAL( true  , schema.validOrder( 0 , 0 ) );
        BOOST_CHECK_EQUAL( true  , schema.validOrder( 0 , 1 ) );
        BOOST_CHECK_EQUAL( false , schema.validOrder( 1 , 0 ) );
    }

    {
        ColumnSchema schema("Name" , Table::DECREASING , Table::DEFAULT_NONE);
        BOOST_CHECK_EQUAL( true  , schema.validOrder( 0 , 0 ) );
        BOOST_CHECK_EQUAL( false , schema.validOrder( 0 , 1 ) );
        BOOST_CHECK_EQUAL( true  , schema.validOrder( 1 , 0 ) );
    }

    {
        ColumnSchema schema("Name" , Table::STRICTLY_INCREASING  , Table::DEFAULT_NONE);
        BOOST_CHECK_EQUAL( false , schema.validOrder( 0 , 0 ) );
        BOOST_CHECK_EQUAL( true  , schema.validOrder( 0 , 1 ) );
        BOOST_CHECK_EQUAL( false , schema.validOrder( 1 , 0 ) );
    }

    {
        ColumnSchema schema("Name" , Table::STRICTLY_DECREASING  , Table::DEFAULT_NONE);
        BOOST_CHECK_EQUAL( false , schema.validOrder( 0 , 0 ) );
        BOOST_CHECK_EQUAL( false , schema.validOrder( 0 , 1 ) );
        BOOST_CHECK_EQUAL( true  , schema.validOrder( 1 , 0 ) );
    }
}


BOOST_AUTO_TEST_CASE( CanLookup ) {
    ColumnSchema schema1("Name" , Table::INCREASING , Table::DEFAULT_NONE);
    ColumnSchema schema2("Name" , Table::RANDOM , Table::DEFAULT_NONE);

    BOOST_CHECK( schema1.lookupValid( ) );
    BOOST_CHECK( !schema2.lookupValid( ) );
}


